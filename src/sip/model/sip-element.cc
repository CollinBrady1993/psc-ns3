/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright 2020 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <sstream>
#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/socket.h"
#include "ns3/boolean.h"
#include "ns3/nstime.h"
#include "sip-element.h"
#include "sip-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SipElement");

namespace sip {

NS_OBJECT_ENSURE_REGISTERED (SipElement);

TypeId
SipElement::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SipElement")
    .SetParent<Object> ()
    .SetGroupName ("Sip")
    .AddConstructor<SipElement> ()
    .AddAttribute ("ReliableTransport",
                   "Whether the transport is reliable (TCP, SCTP) or unreliable (UDP)",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SipElement::m_reliableTransport),
                   MakeBooleanChecker ())
    .AddAttribute ("T1",
                   "RTT Estimate",
                   TimeValue (MilliSeconds (500)), // RFC 3261 default
                   MakeTimeAccessor (&SipElement::m_t1),
                   MakeTimeChecker ())
    .AddAttribute ("T2",
                   "Maximum retransmit interval for non-INVITE requests and INVITE responses",
                   TimeValue (Seconds (4)), // RFC 3261 default
                   MakeTimeAccessor (&SipElement::m_t2),
                   MakeTimeChecker ())
    .AddAttribute ("T4",
                   "Maximum duration a message will remain in the network",
                   TimeValue (Seconds (5)), // RFC 3261 default
                   MakeTimeAccessor (&SipElement::m_t4),
                   MakeTimeChecker ())
    .AddTraceSource ("TxTrace", "The trace for capturing transmitted messages",
                     MakeTraceSourceAccessor (&SipElement::m_txTrace),
                     "ns3::sip::SipElement::TxRxTracedCallback")
    .AddTraceSource ("RxTrace", "The trace for capturing received messages",
                     MakeTraceSourceAccessor (&SipElement::m_rxTrace),
                     "ns3::sip::SipElement::TxRxTracedCallback")
    .AddTraceSource ("DialogState", "Trace of Dialog state changes",
                     MakeTraceSourceAccessor (&SipElement::m_dialogTrace),
                     "ns3::sip::SipElement::DialogStateTracedCallback")
    .AddTraceSource ("TransactionState", "Trace of Transaction state changes",
                     MakeTraceSourceAccessor (&SipElement::m_transactionTrace),
                     "ns3::sip::SipElement::TransactionStateTracedCallback")
  ;
  return tid;
}

SipElement::SipElement ()
  : m_defaultSendCallback (MakeNullCallback<void, Ptr<Packet>, const Address&, const SipHeader&> ())
{
  NS_LOG_FUNCTION (this);
}

SipElement::~SipElement ()
{
  NS_LOG_FUNCTION (this);
  m_dialogs.clear ();
  m_transactions.clear ();
}

void
SipElement::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_receiveCallbacks.clear ();
  m_eventCallbacks.clear ();
  m_defaultSendCallback = MakeNullCallback<void, Ptr<Packet>, const Address&, const SipHeader&> ();
}

std::string
SipElement::TransactionStateToString (TransactionState state)
{
  switch (state)
    {
      case TransactionState::IDLE:
        return "IDLE";
      case TransactionState::CALLING:
        return "CALLING";
      case TransactionState::TRYING:
        return "TRYING";
      case TransactionState::PROCEEDING:
        return "PROCEEDING";
      case TransactionState::COMPLETED:
        return "COMPLETED";
      case TransactionState::CONFIRMED:
        return "CONFIRMED";
      case TransactionState::TERMINATED:
        return "TERMINATED";
      case TransactionState::FAILED:
        return "FAILED";
      default:
        return "Unrecognized state";
    }
}

std::string
SipElement::DialogStateToString (DialogState state)
{
  switch (state)
    {
      case DialogState::UNINITIALIZED:
        return "UNINITIALIZED";
      case DialogState::TRYING:
        return "TRYING";
      case DialogState::PROCEEDING:
        return "PROCEEDING";
      case DialogState::EARLY:
        return "EARLY";
      case DialogState::CONFIRMED:
        return "CONFIRMED";
      case DialogState::TERMINATED:
        return "TERMINATED";
      default:
        return "Unrecognized state";
    }
}

void
SipElement::SendInvite (Ptr<Packet> p, const Address& addr, uint32_t requestUri, uint32_t from, uint32_t to, uint16_t callId, Callback<void, Ptr<Packet>, const Address&, const SipHeader&> sendCallback)
{
  NS_LOG_FUNCTION (p << addr << requestUri << from << to << callId);

  CreateDialog (GetDialogId (callId, from, to), sendCallback);
  SetDialogState (GetDialogId (callId, from, to), DialogState::TRYING);
  TransactionId tid = GetTransactionId (callId, from, to);
  CreateTransaction (tid, sendCallback);
  SetTransactionState (tid, TransactionState::CALLING);
  SipHeader header;
  header.SetMessageType (SipHeader::SIP_REQUEST);
  header.SetMethod (SipHeader::INVITE);
  header.SetRequestUri (requestUri);
  header.SetFrom (from);
  header.SetTo (to);
  header.SetCallId (callId);
  p->AddHeader (header);
  CacheTransactionPacket (tid, p, addr, header);
  sendCallback (p, addr, header);
  m_txTrace (p, header);
  // Start timers A and B
  uint32_t backoff = 1;
  ScheduleTimerA (tid, backoff);
  ScheduleTimerB (tid);
}

void
SipElement::SendBye (Ptr<Packet> p, const Address& addr, uint32_t requestUri, uint32_t from, uint32_t to, uint16_t callId, Callback<void, Ptr<Packet>, const Address&, const SipHeader&> sendCallback)
{
  NS_LOG_FUNCTION (p << addr << requestUri << from << to << callId);
  TransactionId tid = GetTransactionId (callId, from, to);
  DialogId did = GetDialogId (callId, from, to);
  auto it = m_dialogs.find (did);
  NS_ASSERT_MSG (it != m_dialogs.end (), "Dialog not found");
  it->second.m_sendCallback = sendCallback;
  SetDialogState (did, DialogState::TERMINATED);
  if (TransactionExists (tid))
    { 
      SetTransactionState (tid, TransactionState::TRYING);
    }
  else
    { 
      CreateTransaction (tid, sendCallback);
      SetTransactionState (tid, TransactionState::TRYING);
    }
  SipHeader header;
  header.SetMessageType (SipHeader::SIP_REQUEST);
  header.SetMethod (SipHeader::BYE);
  header.SetRequestUri (requestUri);
  header.SetFrom (from);
  header.SetTo (to);
  header.SetCallId (callId);
  p->AddHeader (header);
  CacheTransactionPacket (tid, p, addr, header);
  sendCallback (p, addr, header);
  m_txTrace (p, header);
}

void
SipElement::SendResponse (Ptr<Packet> p, const Address& addr, uint16_t statusCode, uint32_t from, uint32_t to, uint16_t callId, Callback<void, Ptr<Packet>, const Address&, const SipHeader&> sendCallback)
{
  NS_LOG_FUNCTION (p << addr << statusCode << from << to << callId);
  TransactionId tid = GetTransactionId (callId, from, to);
  DialogId did = GetDialogId (callId, from, to);
  auto it = m_dialogs.find (did);
  NS_ASSERT_MSG (it != m_dialogs.end (), "Dialog not found");
  it->second.m_sendCallback = sendCallback;
  if (statusCode == 100)
    {
      SetDialogState (did, DialogState::PROCEEDING);
      SetTransactionState (tid, TransactionState::PROCEEDING);
    }
  else if (statusCode == 200)
    {
      if (it->second.m_state == DialogState::TRYING)
        {
          SetDialogState (did, DialogState::CONFIRMED);
          SetTransactionState (tid, TransactionState::COMPLETED);
        }
      else if (it->second.m_state == DialogState::TERMINATED)
        {
          SetTransactionState (tid, TransactionState::COMPLETED);
          // Set Timer
        }
    }
  SipHeader header;
  header.SetMessageType (SipHeader::SIP_RESPONSE);
  header.SetStatusCode (statusCode);
  header.SetFrom (from);
  header.SetTo (to);
  header.SetCallId (callId);
  p->AddHeader (header);
  sendCallback (p, addr, header);
  m_txTrace (p, header);
}

void
SipElement::Receive (Ptr<Packet> p, Address from)
{
  NS_LOG_FUNCTION (this << p << from);
  SipHeader sipHeader;
  p->RemoveHeader (sipHeader);
  m_rxTrace (p, sipHeader);
  TransactionId tid = GetTransactionId (sipHeader.GetCallId (), sipHeader.GetFrom (), sipHeader.GetTo ());
  NS_LOG_DEBUG ("Receive packet for TransactionId " << TransactionIdToString (tid));
  DialogId did = GetDialogId (sipHeader.GetCallId (), sipHeader.GetFrom (), sipHeader.GetTo ());
  auto eventIt = m_eventCallbacks.find (sipHeader.GetCallId ());
  auto receiveIt = m_receiveCallbacks.find (sipHeader.GetCallId ());
  if (eventIt == m_eventCallbacks.end () || receiveIt == m_receiveCallbacks.end ())
    {
      NS_FATAL_ERROR ("CallId does not have callbacks set");
    }
  if (sipHeader.GetMessageType () == SipHeader::SIP_RESPONSE)
    {
      if (sipHeader.GetStatusCode () == 100)
        { 
          NS_LOG_DEBUG ("Received 100 Trying for call ID " << sipHeader.GetCallId ());
          eventIt->second (TRYING_RECEIVED, TransactionState::PROCEEDING);
          SetDialogState (did, DialogState::PROCEEDING);
          SetTransactionState (tid, TransactionState::PROCEEDING);
          CancelTimerA (tid);
          CancelTimerB (tid);
          FreeTransactionPacket (tid);
        }
      else if (sipHeader.GetStatusCode () == 200)
        { 
          NS_LOG_DEBUG ("Received 200 OK for call ID " << sipHeader.GetCallId ());
          auto dialogIt = m_dialogs.find (did);
          if (dialogIt->second.m_state == DialogState::TRYING || dialogIt->second.m_state == DialogState::PROCEEDING)
            {
              SetDialogState (did, DialogState::CONFIRMED);
              CancelTimerA (tid);
              CancelTimerB (tid);
              SetTransactionState (tid, TransactionState::TERMINATED);
              FreeTransactionPacket (tid);
              // Deliver the packet since the OK may have SDP information
              receiveIt->second (p, sipHeader, TransactionState::TERMINATED);
              // Start Timer I
              NS_LOG_DEBUG ("Send ACK for call ID " << sipHeader.GetCallId ());
              Ptr<Packet> packet = Create<Packet> ();
              SipHeader header;
              header.SetMessageType (SipHeader::SIP_REQUEST);
              header.SetMethod (SipHeader::ACK);
              header.SetRequestUri (sipHeader.GetRequestUri ());
              header.SetFrom (sipHeader.GetFrom ());
              header.SetTo (sipHeader.GetTo ());
              header.SetCallId (sipHeader.GetCallId ());
              packet->AddHeader (header);
              // ACK to the source address of the incoming packet.
              dialogIt->second.m_sendCallback (packet, from, header);
              m_txTrace (packet, header);
            }
          else if (dialogIt->second.m_state == DialogState::CONFIRMED)
            {
              // The transaction should be already terminated, but possibly the
              // ACK was lost.
              NS_LOG_DEBUG ("Resend ACK for call ID " << sipHeader.GetCallId ());
              Ptr<Packet> packet = Create<Packet> ();
              SipHeader header;
              header.SetMessageType (SipHeader::SIP_REQUEST);
              header.SetMethod (SipHeader::ACK);
              header.SetRequestUri (sipHeader.GetRequestUri ());
              header.SetFrom (sipHeader.GetFrom ());
              header.SetTo (sipHeader.GetTo ());
              header.SetCallId (sipHeader.GetCallId ());
              packet->AddHeader (header);
              // ACK to the source address of the incoming packet.
              dialogIt->second.m_sendCallback (packet, from, header);
              m_txTrace (packet, header);
            }
          else if (dialogIt->second.m_state == DialogState::TERMINATED)
            {
              NS_LOG_DEBUG ("No ACK needed for 200 OK response to BYE");
              SetTransactionState (tid, TransactionState::COMPLETED);
              // Deliver the packet, although the OK of BYE should not include SDP
              receiveIt->second (p, sipHeader, TransactionState::COMPLETED);
              // Set Timer K to transition to TERMINATED
            }
          else
            {
              NS_FATAL_ERROR ("Received 200 OK in unexpected state");
            }
        }
    }
  else if (sipHeader.GetMessageType () == SipHeader::SIP_REQUEST)
    {
      if (sipHeader.GetMethod () == SipHeader::INVITE)
        { 
          NS_LOG_DEBUG ("Received INVITE for call ID " << sipHeader.GetCallId ());
          auto dialogIt = m_dialogs.find (did);
          if (dialogIt == m_dialogs.end ())
            {
              NS_LOG_DEBUG ("Creating dialog for call ID " << sipHeader.GetCallId ());
              CreateDialog (did, m_defaultSendCallback);
              SetDialogState (did, DialogState::TRYING);
              CreateTransaction (tid, m_defaultSendCallback);
              SetTransactionState (tid, TransactionState::TRYING);
              receiveIt->second (p, sipHeader, TransactionState::TRYING);
            }
          else
            {
              NS_LOG_DEBUG ("Dialog already exists; ignoring possible retransmission");
            }
        }
      else if (sipHeader.GetMethod () == SipHeader::BYE)
        { 
          NS_LOG_DEBUG ("Received BYE for call ID " << sipHeader.GetCallId ());
          SetDialogState (did, DialogState::TERMINATED);
          if (!TransactionExists (tid))
            {
              CreateTransaction (tid, m_defaultSendCallback);
            }
          SetTransactionState (tid, TransactionState::TRYING);
          receiveIt->second (p, sipHeader, TransactionState::TRYING);
        }
      else if (sipHeader.GetMethod () == SipHeader::ACK)
        { 
          NS_LOG_DEBUG ("Received ACK for call ID " << sipHeader.GetCallId ());
          eventIt->second (ACK_RECEIVED, TransactionState::CONFIRMED);
          SetTransactionState (tid, TransactionState::CONFIRMED);
          // Stop Timer H
          // Start Timer I (to absorb any acks)
        }
    }
}

void
SipElement::SetCallbacks (uint16_t callId, Callback<void, Ptr<Packet>, const SipHeader&, TransactionState> receiveCallback, Callback<void, const char*, TransactionState> eventCallback)
{
  NS_LOG_FUNCTION (this << callId);

  auto element = m_receiveCallbacks.find (callId);
  if (element == m_receiveCallbacks.end ())
    {
      m_receiveCallbacks.emplace (callId, receiveCallback);
    }
  else
    {
      NS_FATAL_ERROR ("CallId has already configured a receive callback");
    }
  auto element2 = m_eventCallbacks.find (callId);
  if (element2 == m_eventCallbacks.end ())
    {
      m_eventCallbacks.emplace (callId, eventCallback);
    }
  else
    {
      NS_FATAL_ERROR ("CallId has already configured an event callback");
    }
}

void
SipElement::SetDefaultSendCallback (Callback<void, Ptr<Packet>, const Address&, const SipHeader&> sendCallback)
{
  NS_LOG_FUNCTION (this);
  m_defaultSendCallback = sendCallback;
}

// Protected members 

std::string
SipElement::DialogIdToString (DialogId id) const
{
  std::stringstream ss;
  ss << "(" << std::get<0> (id) << "," << std::get<1> (id) << "," << std::get<2> (id) << ")";
  return ss.str ();
}

SipElement::DialogId
SipElement::GetDialogId (uint16_t callId, uint32_t uriA, uint32_t uriB) const
{
  if (uriA < uriB)
    {
      return DialogId (callId, uriA, uriB);
    }
  else
    {
      return DialogId (callId, uriB, uriA);
    }
}

void
SipElement::CreateDialog (DialogId id, Callback<void, Ptr<Packet>, const Address&, const SipHeader&> sendCallback)
{
  NS_LOG_FUNCTION (this << DialogIdToString (id));
  Dialog dialog (std::get<0> (id), sendCallback, DialogState::UNINITIALIZED);
  std::pair<std::unordered_map<DialogId, Dialog, TupleHash>::iterator, bool> returnValue;
  returnValue = m_dialogs.emplace (id, dialog);
  NS_ABORT_MSG_UNLESS (returnValue.second, "Emplace SipElement Dialog failed");
}

bool
SipElement::DialogExists (DialogId id) const
{
  auto dialogIt = m_dialogs.find (id);
  if (dialogIt == m_dialogs.end ())
    {
      return false;
    }
  else
    {
      return true;
    }
}

void
SipElement::SetDialogState (DialogId id, DialogState state)
{
  NS_LOG_FUNCTION (this << DialogIdToString (id) << DialogStateToString (state));
  auto dialogIt = m_dialogs.find (id);
  NS_ABORT_MSG_IF (dialogIt == m_dialogs.end (), "Dialog not found");
  dialogIt->second.m_state = state;
  m_dialogTrace (std::get<0> (id), std::get<1> (id), std::get<2> (id), state);
}

std::string
SipElement::TransactionIdToString (TransactionId id) const
{
  std::stringstream ss;
  ss << "(" << std::get<0> (id) << "," << std::get<1> (id) << "," << std::get<2> (id) << ")";
  return ss.str ();
}

SipElement::TransactionId
SipElement::GetTransactionId (uint16_t callId, uint32_t from, uint32_t to) const
{
  return TransactionId (callId, from, to);
}

void
SipElement::CreateTransaction (TransactionId id, Callback<void, Ptr<Packet>, const Address&, const SipHeader&> sendCallback)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id));
  auto transIt = m_transactions.find (id);
  if (transIt == m_transactions.end ())
    {
      std::pair<std::unordered_map<TransactionId, Transaction, TupleHash>::iterator, bool> returnValue;
      returnValue = m_transactions.emplace (id, Transaction (std::get<0> (id), sendCallback));
      NS_ABORT_MSG_UNLESS (returnValue.second, "Emplace SipElement Transaction failed");
    }
  else
    {
      transIt->second.m_state = TransactionState::IDLE;
    }
}

bool
SipElement::TransactionExists (TransactionId id) const
{
  auto transIt = m_transactions.find (id);
  if (transIt == m_transactions.end ())
    {
      return false;
    }
  else
    {
      return true;
    }
}

void
SipElement::SetTransactionState (TransactionId id, TransactionState state)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id) << TransactionStateToString (state));
  auto transIt = m_transactions.find (id);
  NS_ABORT_MSG_IF (transIt == m_transactions.end (), "Transaction not found");
  transIt->second.m_state = state;
  m_transactionTrace (std::get<0> (id), std::get<1> (id), std::get<2> (id), state);
}

void
SipElement::CacheTransactionPacket (TransactionId id, Ptr<const Packet> p, const Address& addr, const SipHeader& hdr)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id));
  auto transIt = m_transactions.find (id);
  NS_ABORT_MSG_IF (transIt == m_transactions.end (), "Transaction not found");
  transIt->second.m_packet = p->Copy ();
  transIt->second.m_address = addr;
  transIt->second.m_sipHeader = hdr;
}

Ptr<const Packet>
SipElement::GetTransactionPacket (TransactionId id)
{
  auto transIt = m_transactions.find (id);
  NS_ABORT_MSG_IF (transIt == m_transactions.end (), "Transaction not found");
  return transIt->second.m_packet;
}

void
SipElement::FreeTransactionPacket (TransactionId id)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id));
  auto transIt = m_transactions.find (id);
  if (transIt != m_transactions.end ())
    {
      transIt->second.m_packet = 0;
    }
}

void
SipElement::ScheduleTimerA (TransactionId id, uint32_t backoff)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id) << backoff);
  auto transIt = m_transactions.find (id);
  NS_ASSERT_MSG (transIt != m_transactions.end (), "Transaction not found");
  transIt->second.m_timerA.SetFunction (&SipElement::HandleTimerA, this);
  transIt->second.m_timerA.SetArguments (id, backoff);
  transIt->second.m_timerA.Schedule (backoff * m_t1);
}

void
SipElement::CancelTimerA (TransactionId id)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id));
  auto transIt = m_transactions.find (id);
  NS_ASSERT_MSG (transIt != m_transactions.end (), "Transaction not found");
  transIt->second.m_timerA.Cancel ();
}

void
SipElement::ScheduleTimerB (TransactionId id)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id));
  auto transIt = m_transactions.find (id);
  NS_ASSERT_MSG (transIt != m_transactions.end (), "Transaction not found");
  transIt->second.m_timerB.SetFunction (&SipElement::HandleTimerB, this);
  transIt->second.m_timerB.SetArguments (id);
  transIt->second.m_timerB.Schedule (64 * m_t1);
}

void
SipElement::CancelTimerB (TransactionId id)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id));
  auto transIt = m_transactions.find (id);
  NS_ASSERT_MSG (transIt != m_transactions.end (), "Transaction not found");
  transIt->second.m_timerB.Cancel ();
}

void
SipElement::HandleTimerA (TransactionId id, uint32_t backoff)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id) << backoff);
  auto eventIt = m_eventCallbacks.find (std::get<0> (id));
  NS_ASSERT_MSG (eventIt != m_eventCallbacks.end (), "CallID not found");
  auto transIt = m_transactions.find (id);
  NS_ASSERT_MSG (transIt != m_transactions.end (), "Transaction not found");
  NS_ASSERT_MSG (transIt->second.m_state == TransactionState::CALLING, "Transaction not in CALLING");
  eventIt->second (TIMER_A_EXPIRED, transIt->second.m_state);
  // Resend the cached packet
  transIt->second.m_sendCallback (transIt->second.m_packet, transIt->second.m_address, transIt->second.m_sipHeader);
  // Double the backoff as a multiplier of T1, and reschedule
  backoff = backoff << 1;
  ScheduleTimerA (id, backoff);
}

void
SipElement::HandleTimerB (TransactionId id)
{
  NS_LOG_FUNCTION (this << TransactionIdToString (id));
  DialogId did = GetDialogId (std::get<0> (id), std::get<1> (id), std::get<2> (id));
  auto eventIt = m_eventCallbacks.find (std::get<0> (id));
  NS_ASSERT_MSG (eventIt != m_eventCallbacks.end (), "CallID not found");
  auto transIt = m_transactions.find (id);
  NS_ASSERT_MSG (transIt != m_transactions.end (), "Transaction not found");
  NS_ASSERT_MSG (transIt->second.m_state == TransactionState::CALLING, "Transaction not in CALLING");
  eventIt->second (TIMER_B_EXPIRED, transIt->second.m_state);
  // Cancel timer A and fail the transaction 
  CancelTimerA (id);
  SetTransactionState (id, TransactionState::FAILED);
  SetDialogState (did, DialogState::TERMINATED);
}

std::ostream& operator<< (std::ostream& os, const SipElement::DialogState& state)
{
   os << static_cast<uint16_t> (state);
   return os;
}

std::ostream& operator<< (std::ostream& os, const SipElement::TransactionState& state)
{
   os << static_cast<uint16_t> (state);
   return os;
}

} // namespace sip

} // namespace ns3
