/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * NIST-developed software is provided by NIST as a public service. You may
 * use, copy and distribute copies of the software in any medium, provided that
 * you keep intact this entire notice. You may improve, modify and create
 * derivative works of the software or any portion of the software, and you may
 * copy and distribute such modifications or works. Modified works should carry
 * a notice stating that you changed the software and should note the date and
 * nature of any such change. Please explicitly acknowledge the National
 * Institute of Standards and Technology as the source of the software.
 * 
 * NIST-developed software is expressly provided "AS IS." NIST MAKES NO
 * WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
 * LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST
 * NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE
 * UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST
 * DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 * SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 * CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 * 
 * You are solely responsible for determining the appropriateness of using and
 * distributing the software and you assume all risks associated with its use,
 * including but not limited to the risks and costs of program errors,
 * compliance with applicable laws, damage to or loss of data, programs or
 * equipment, and the unavailability or interruption of operation. This
 * software is not intended to be used in any situation where a failure could
 * cause risk of injury or damage to property. The software developed by NIST
 * employees is not subject to copyright protection within the United States.
 */

#include <ns3/callback.h>
#include <ns3/log.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/pointer.h>
#include <ns3/ptr.h>
#include <ns3/type-id.h>
#include <ns3/sip-header.h>

#include "mcptt-call-machine.h"
#include "mcptt-call-msg.h"
#include "mcptt-chan.h"
#include "mcptt-floor-participant.h"
#include "mcptt-floor-msg.h"
#include "mcptt-media-msg.h"
#include "mcptt-on-network-call-machine-client.h"
#include "mcptt-ptt-app.h"

#include "mcptt-call.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("McpttCall");

NS_OBJECT_ENSURE_REGISTERED (McpttCall);

TypeId
McpttCall::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::McpttCall")
    .SetParent<Object> ()
    .AddConstructor<McpttCall> ()
    .AddAttribute ("CallMachine", "The call machine of the call.",
                   PointerValue (0),
                   MakePointerAccessor (&McpttCall::m_callMachine),
                   MakePointerChecker<McpttCallMachine> ())
    .AddAttribute ("FloorMachine", "The floor machine of the call.",
                   PointerValue (0),
                   MakePointerAccessor (&McpttCall::m_floorMachine),
                   MakePointerChecker<McpttFloorParticipant> ())
  ;
  return tid;
}

McpttCall::McpttCall (void)
  : Object (),
    m_floorChan (0),
    m_mediaChan (0),
    m_owner (0),
    m_pushOnSelect (false),
    m_startTime (Seconds (0)),
    m_stopTime (Seconds (0)),
    m_rxCb (MakeNullCallback<void, Ptr<const McpttCall>, const Header&> ()),
    m_txCb (MakeNullCallback<void, Ptr<const McpttCall>, const Header&> ())
{
  NS_LOG_FUNCTION (this);
}

McpttCall::~McpttCall (void)
{
  NS_LOG_FUNCTION (this);
}

void
McpttCall::CloseFloorChan (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<McpttChan> floorChan = GetFloorChan ();
  floorChan->Close ();
}

void
McpttCall::CloseMediaChan (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<McpttChan> mediaChan = GetMediaChan ();
  mediaChan->Close ();
}

void
McpttCall::SetCallId (uint16_t callId)
{
  NS_LOG_DEBUG (this << callId);
  if (m_callMachine)
    {
      m_callMachine->SetCallId (callId);
    }
  else
    {
      NS_FATAL_ERROR ("Error:  setting call ID on a call without a call machine");
    }
}

uint16_t
McpttCall::GetCallId (void) const
{
  if (m_callMachine)
    {
      return m_callMachine->GetCallId ().GetCallId ();
    }
  else
    {
      return 0;
    }
}

bool
McpttCall::GetPushOnSelect (void) const
{
  return m_pushOnSelect;
}

void
McpttCall::SetPushOnSelect (bool pushOnSelect)
{
  m_pushOnSelect = pushOnSelect;
}

bool
McpttCall::IsFloorChanOpen (void) const
{
  Ptr<McpttChan> floorChan = GetFloorChan ();
  bool isOpen = floorChan->IsOpen ();

  return isOpen;
}

bool
McpttCall::IsMediaChanOpen (void) const
{
  Ptr<McpttChan> mediaChan = GetMediaChan ();
  bool isOpen = mediaChan->IsOpen ();

  return isOpen;
}

void
McpttCall::OpenFloorChan (const Address& peerAddr, const uint16_t port)
{
  NS_LOG_FUNCTION (this << peerAddr << port);

  Ptr<McpttPttApp> owner = GetOwner ();
  Ptr<Node> node = owner->GetNode ();
  Ptr<McpttChan> floorChan = GetFloorChan ();
  Address localAddr = owner->GetLocalAddress ();

  floorChan->Open (node, port, localAddr, peerAddr);
}

void
McpttCall::OpenMediaChan (const Address& peerAddr, const uint16_t port)
{
  NS_LOG_FUNCTION (this << peerAddr << port);

  Ptr<McpttPttApp> owner = GetOwner ();
  Ptr<Node> node = owner->GetNode ();
  Ptr<McpttChan> mediaChan = GetMediaChan ();
  Address localAddr = owner->GetLocalAddress ();

  mediaChan->Open (node, port, localAddr, peerAddr);
}

void
McpttCall::Receive (const McpttCallMsg& msg)
{
  NS_LOG_FUNCTION (this << & msg);

  if (!m_rxCb.IsNull ())
    {
      m_rxCb (this, msg);
    }

  Ptr<McpttCallMachine> callMachine = GetCallMachine ();
  callMachine->Receive (msg);
}

void
McpttCall::Receive (Ptr<Packet> pkt, const SipHeader& hdr)
{
  NS_LOG_FUNCTION (this << pkt);
  NS_ASSERT_MSG (hdr.GetCallId () == GetCallId (), "Received message for wrong call ID");
  NS_LOG_DEBUG ("Received SIP packet for call ID " << GetCallId ());
  if (!m_rxCb.IsNull ())
    {
      m_rxCb (this, hdr);
    }
  GetCallMachine ()->GetObject<McpttOnNetworkCallMachineClient> ()->ReceiveCallPacket (pkt, hdr);
}

void
McpttCall::Receive (const McpttFloorMsg& msg)
{
  NS_LOG_FUNCTION (this << &msg);

  if (!m_rxCb.IsNull ())
    {
      m_rxCb (this, msg);
    }

  Ptr<McpttFloorParticipant> floorMachine = GetFloorMachine ();
  floorMachine->Receive (msg);
}

void
McpttCall::Receive (const McpttMediaMsg& msg)
{
  NS_LOG_FUNCTION (this << &msg);

  if (!m_rxCb.IsNull ())
    {
      m_rxCb (this, msg);
    }

  Ptr<McpttCallMachine> callMachine = GetCallMachine ();
  Ptr<McpttFloorParticipant> floorMachine = GetFloorMachine ();

  callMachine->Receive (msg);
  floorMachine->Receive (msg);
}

void
McpttCall::Send (Ptr<Packet> pkt, const SipHeader& hdr)
{
  NS_LOG_FUNCTION (this << hdr);

  if (!m_txCb.IsNull ())
    {
      m_txCb (this, hdr);
    }

  GetOwner ()->Send (pkt, hdr);
}

void
McpttCall::Send (const McpttCallMsg& msg)
{
  NS_LOG_FUNCTION (this << &msg);

  if (!m_txCb.IsNull ())
    {
      m_txCb (this, msg);
    }

  Ptr<McpttPttApp> parent = GetOwner ();
  parent->Send (msg);
}

void
McpttCall::Send (const McpttFloorMsg& msg)
{
  NS_LOG_FUNCTION (this << &msg);

  if (!m_txCb.IsNull ())
    {
      m_txCb (this, msg);
    }

  Ptr<Packet> pkt = Create<Packet> ();
  Ptr<McpttChan> floorChan = GetFloorChan ();

  pkt->AddHeader (msg);

  floorChan->Send (pkt);
}

void
McpttCall::Send (const McpttMediaMsg& msg)
{
  NS_LOG_FUNCTION (this << &msg);

  Ptr<Packet> pkt = Create<Packet> ();
  Ptr<McpttChan> mediaChan = GetMediaChan ();
  Ptr<McpttFloorParticipant> floorMachine = GetFloorMachine ();

  McpttMediaMsg txMsg (msg);

  floorMachine->MediaReady (txMsg);

  if (!m_txCb.IsNull ())
    {
      m_txCb (this, msg);
    }

  pkt->AddHeader (txMsg);

  mediaChan->Send (pkt);
}

void
McpttCall::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  SetCallMachine (0);
  SetFloorChan (0);
  SetFloorMachine (0);
  SetMediaChan (0);
  SetOwner (0);

  Object::DoDispose ();
}

void
McpttCall::ReceiveFloorPkt (Ptr<Packet>  pkt)
{
  NS_LOG_FUNCTION (this << &pkt);

  McpttFloorMsg temp;

  pkt->PeekHeader (temp);
  uint8_t subtype = temp.GetSubtype ();

  if (subtype == McpttFloorMsgRequest::SUBTYPE)
    {
      McpttFloorMsgRequest reqMsg;
      pkt->RemoveHeader (reqMsg);
      Receive (reqMsg);
    }
  else if (subtype == McpttFloorMsgGranted::SUBTYPE
      || subtype == McpttFloorMsgGranted::SUBTYPE_ACK)
    {
      McpttFloorMsgGranted grantedMsg;
      pkt->RemoveHeader (grantedMsg);
      Receive (grantedMsg);
    }
  else if (subtype == McpttFloorMsgDeny::SUBTYPE
      || subtype == McpttFloorMsgDeny::SUBTYPE_ACK)
    {
      McpttFloorMsgDeny denyMsg;
      pkt->RemoveHeader (denyMsg);
      Receive (denyMsg);
    }
  else if (subtype == McpttFloorMsgRelease::SUBTYPE
      || subtype == McpttFloorMsgRelease::SUBTYPE_ACK)
    {
      McpttFloorMsgRelease releaseMsg;
      pkt->RemoveHeader (releaseMsg);
      Receive (releaseMsg);
    }
  else if (subtype == McpttFloorMsgIdle::SUBTYPE
      || subtype == McpttFloorMsgIdle::SUBTYPE_ACK)
    {
      McpttFloorMsgIdle idleMsg;
      pkt->RemoveHeader (idleMsg);
      Receive (idleMsg);
    }
  else if (subtype == McpttFloorMsgTaken::SUBTYPE
      || subtype == McpttFloorMsgTaken::SUBTYPE_ACK)
    {
      McpttFloorMsgTaken takenMsg;
      pkt->RemoveHeader (takenMsg);
      Receive (takenMsg);
    }
  else if (subtype == McpttFloorMsgRevoke::SUBTYPE)
    {
      McpttFloorMsgRevoke revokeMsg;
      pkt->RemoveHeader (revokeMsg);
      Receive (revokeMsg);
    }
  else if (subtype == McpttFloorMsgQueuePositionRequest::SUBTYPE)
    {
      McpttFloorMsgQueuePositionRequest queuePositionRequestMsg;
      pkt->RemoveHeader (queuePositionRequestMsg);
      Receive (queuePositionRequestMsg);
    }
  else if (subtype == McpttFloorMsgQueuePositionInfo::SUBTYPE
      || subtype == McpttFloorMsgQueuePositionInfo::SUBTYPE_ACK)
    {
      McpttFloorMsgQueuePositionInfo queueInfoMsg;
      pkt->RemoveHeader (queueInfoMsg);
      Receive (queueInfoMsg);
    }
  else if (subtype == McpttFloorMsgAck::SUBTYPE)
    {
      McpttFloorMsgAck ackMsg;
      pkt->RemoveHeader (ackMsg);
      Receive (ackMsg);
    }
  else
    {
      NS_FATAL_ERROR ("Could not resolve message subtype = " << (uint32_t)subtype << ".");
    }
}

void
McpttCall::ReceiveMediaPkt (Ptr<Packet>  pkt)
{
  NS_LOG_FUNCTION (this << &pkt);

  McpttMediaMsg msg;

  pkt->RemoveHeader (msg);

  Receive (msg);
}

Ptr<McpttCallMachine>
McpttCall::GetCallMachine (void) const
{
  return m_callMachine;
}

Ptr<McpttChan>
McpttCall::GetFloorChan (void) const
{
  return m_floorChan;
}

Ptr<McpttFloorParticipant>
McpttCall::GetFloorMachine (void) const
{
  return m_floorMachine;
}

Ptr<McpttChan>
McpttCall::GetMediaChan (void) const
{
  return m_mediaChan;
}

Ptr<McpttPttApp>
McpttCall::GetOwner (void) const
{
  return m_owner;
}

Time
McpttCall::GetStartTime (void) const
{
  return m_startTime;
}

Time
McpttCall::GetStopTime (void) const
{
  return m_stopTime;
}

void
McpttCall::SetCallMachine (Ptr<McpttCallMachine>  callMachine)
{
  NS_LOG_FUNCTION (this << &callMachine);

  if (callMachine != 0)
    {
      callMachine->SetOwner (this);
    }

  m_callMachine = callMachine;
}

void
McpttCall::SetFloorChan (Ptr<McpttChan>  floorChan)
{
  NS_LOG_FUNCTION (this << &floorChan);

  if (floorChan != 0)
    {
      floorChan->SetRxPktCb (MakeCallback (&McpttCall::ReceiveFloorPkt, this));
    }

  m_floorChan = floorChan;
}

void
McpttCall::SetFloorMachine (Ptr<McpttFloorParticipant>  floorMachine)
{
  NS_LOG_FUNCTION (this << &floorMachine);

  if (floorMachine != 0)
    {
      floorMachine->SetOwner (this);
    }

  m_floorMachine = floorMachine;
}

void
McpttCall::SetMediaChan (Ptr<McpttChan>  mediaChan)
{
  NS_LOG_FUNCTION (this << &mediaChan);

  if (mediaChan != 0)
    {
      mediaChan->SetRxPktCb (MakeCallback (&McpttCall::ReceiveMediaPkt, this));
    }

  m_mediaChan = mediaChan;
}

void
McpttCall::SetOwner (Ptr<McpttPttApp> owner)
{
  NS_LOG_FUNCTION (this << owner);

  m_owner = owner;
}

void
McpttCall::SetRxCb (const Callback<void, Ptr<const McpttCall>, const Header&>  rxCb)
{
  NS_LOG_FUNCTION (this);

  m_rxCb = rxCb;
}

void
McpttCall::SetTxCb (const Callback<void, Ptr<const McpttCall>, const Header&>  txCb)
{
  NS_LOG_FUNCTION (this << &txCb);

  m_txCb = txCb;
}

void
McpttCall::SetStartTime (Time startTime)
{
  NS_LOG_FUNCTION (this << startTime);
  m_startTime = startTime;
}

void
McpttCall::SetStopTime (Time stopTime)
{
  NS_LOG_FUNCTION (this << stopTime);
  m_stopTime = stopTime;
}

} // namespace ns3

