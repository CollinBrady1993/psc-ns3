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

#include <ns3/boolean.h>
#include <ns3/log.h>
#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/type-id.h>

#include "mcptt-counter.h"
#include "mcptt-call-control-info.h"
#include "mcptt-floor-msg.h"
#include "mcptt-floor-msg-sink.h"
#include "mcptt-media-msg.h"
#include "mcptt-on-network-floor-arbitrator-state.h"
#include "mcptt-on-network-floor-server-app.h"
#include "mcptt-on-network-floor-towards-participant.h"
#include "mcptt-ptt-app.h"
#include "mcptt-timer.h"

#include "mcptt-on-network-floor-arbitrator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("McpttOnNetworkFloorArbitrator");

/** McpttOnNetworkFloorArbitrator - begin **/
NS_OBJECT_ENSURE_REGISTERED (McpttOnNetworkFloorArbitrator);

TypeId
McpttOnNetworkFloorArbitrator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::McpttOnNetworkFloorArbitrator")
    .SetParent<Object> ()
    .AddConstructor<McpttOnNetworkFloorArbitrator>()
    .AddAttribute ("AckRequired", "The flag that indicates if acknowledgement is required.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&McpttOnNetworkFloorArbitrator::m_ackRequired),
                   MakeBooleanChecker ())
    .AddAttribute ("AudioCutIn", "The flag that indicates if the group is configured for audio cut-in.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&McpttOnNetworkFloorArbitrator::m_audioCutIn),
                   MakeBooleanChecker ())
    .AddAttribute ("C7", "The initial limit of counter C7.",
                   UintegerValue (10),
                   MakeUintegerAccessor (&McpttOnNetworkFloorArbitrator::SetLimitC7),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("C20", "The initial limit of counter C20.",
                   UintegerValue (3),
                   MakeUintegerAccessor (&McpttOnNetworkFloorArbitrator::SetLimitC20),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DualFloorSupported", "The flag that indicates if dual floor control is supported.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&McpttOnNetworkFloorArbitrator::m_dualFloorSupported),
                   MakeBooleanChecker ())
    .AddAttribute ("McGranted", "The flag that indicates if the \"mc_granted\" fmtp attribute was included",
                   BooleanValue (false),
                   MakeBooleanAccessor (&McpttOnNetworkFloorArbitrator::m_mcGranted),
                   MakeBooleanChecker ())
    .AddAttribute ("TxSsrc", "The SSRC to use when transmitting a message.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&McpttOnNetworkFloorArbitrator::m_txSsrc),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("T1", "The delay to use for timer T1 (Time value)",
                   TimeValue (Seconds (4)),
                   MakeTimeAccessor (&McpttOnNetworkFloorArbitrator::SetDelayT1),
                   MakeTimeChecker ())
    .AddAttribute ("T2", "The delay to use for timer T2 (Time value)",
                   TimeValue (Seconds (30)),
                   MakeTimeAccessor (&McpttOnNetworkFloorArbitrator::SetDelayT2),
                   MakeTimeChecker ())
    .AddAttribute ("T3", "The delay to use for timer T3 (Time value)",
                   TimeValue (Seconds (3)),
                   MakeTimeAccessor (&McpttOnNetworkFloorArbitrator::SetDelayT3),
                   MakeTimeChecker ())
    .AddAttribute ("T4", "The delay to use for timer T4 (Time value)",
                   TimeValue (Seconds (30)),
                   MakeTimeAccessor (&McpttOnNetworkFloorArbitrator::SetDelayT4),
                   MakeTimeChecker ())
    .AddAttribute ("T7", "The delay to use for timer T7 (Time value)",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&McpttOnNetworkFloorArbitrator::SetDelayT7),
                   MakeTimeChecker ())
    .AddAttribute ("T20", "The delay to use for timer T20 (Time value)",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&McpttOnNetworkFloorArbitrator::SetDelayT20),
                   MakeTimeChecker ())
    .AddTraceSource ("StateChangeTrace",
                   "The trace for capturing state changes.",
                   MakeTraceSourceAccessor (&McpttOnNetworkFloorArbitrator::m_stateChangeTrace),
                   "ns3::McpttOnNetworkFloorArbitrator::StateChangeTrace")
    ;
  
  return tid;
}

McpttOnNetworkFloorArbitrator::McpttOnNetworkFloorArbitrator (void)
  : Object (),
    m_c7 (CreateObject<McpttCounter> (McpttEntityId (7, "C7"))),
    m_c20 (CreateObject<McpttCounter> (McpttEntityId (20, "C20"))),
    m_dualFloorSupported (false),
    m_dualControl (CreateObject<McpttOnNetworkFloorDualControl> ()),
    m_owner (0),
    m_participants (std::vector<Ptr<McpttOnNetworkFloorTowardsParticipant> >()),
    m_queue (CreateObject<McpttFloorQueue> ()),
    m_rejectCause (0),
    m_rxCb (MakeNullCallback<void, const McpttFloorMsg&> ()),
    m_seqNum (0),
    m_state (McpttOnNetworkFloorArbitratorStateStartStop::GetInstance ()),
    m_stateChangeCb (MakeNullCallback<void, const McpttEntityId&, const McpttEntityId&> ()),
    m_storedSsrc (0),
    m_storedPriority (0),
    m_trackInfo (McpttFloorMsgFieldTrackInfo ()),
    m_t1 (CreateObject<McpttTimer> (McpttEntityId (1, "T1"))),
    m_t2 (CreateObject<McpttTimer> (McpttEntityId (2, "T2"))),
    m_t3 (CreateObject<McpttTimer> (McpttEntityId (3, "T3"))),
    m_t4 (CreateObject<McpttTimer> (McpttEntityId (4, "T4"))),
    m_t7 (CreateObject<McpttTimer> (McpttEntityId (7, "T7"))),
    m_t20 (CreateObject<McpttTimer> (McpttEntityId (20, "T20"))),
    m_txCb (MakeNullCallback<void, const McpttFloorMsg&> ())
{
  NS_LOG_FUNCTION (this);

  m_dualControl->SetOwner (this);

  m_t1->Link (&McpttOnNetworkFloorArbitrator::ExpiryOfT1, this);
  m_t2->Link (&McpttOnNetworkFloorArbitrator::ExpiryOfT2, this);
  m_t3->Link (&McpttOnNetworkFloorArbitrator::ExpiryOfT3, this);
  m_t4->Link (&McpttOnNetworkFloorArbitrator::ExpiryOfT4, this);
  m_t7->Link (&McpttOnNetworkFloorArbitrator::ExpiryOfT7, this);
  m_t20->Link (&McpttOnNetworkFloorArbitrator::ExpiryOfT20, this);
}

McpttOnNetworkFloorArbitrator::~McpttOnNetworkFloorArbitrator (void)
{
  NS_LOG_FUNCTION (this);
}

void
McpttOnNetworkFloorArbitrator::AddParticipant (Ptr<McpttOnNetworkFloorTowardsParticipant> participant)
{
  NS_LOG_FUNCTION (this);

  participant->SetOwner (this);
  m_participants.push_back (participant);
}

void
McpttOnNetworkFloorArbitrator::CallInitialized (McpttOnNetworkFloorTowardsParticipant& participant)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") call initialized.");

  m_state->CallInitialized (*this, participant);
}

void
McpttOnNetworkFloorArbitrator::CallRelease1 (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") call released (part I).");

  m_state->CallRelease1 (*this);
}

void
McpttOnNetworkFloorArbitrator::CallRelease2 (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") call released (part II).");

  m_state->CallRelease2 (*this);
}

void
McpttOnNetworkFloorArbitrator::ClientRelease (void)
{
  NS_LOG_FUNCTION (this);
  
  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") taking client release notification.");

  m_state->ClientRelease (*this);
}

void
McpttOnNetworkFloorArbitrator::ChangeState (Ptr<McpttOnNetworkFloorArbitratorState>  state)
{
  NS_LOG_FUNCTION (this << state);

  McpttEntityId stateId = state->GetInstanceStateId ();

  McpttEntityId currStateId = GetStateId ();

  if (currStateId != stateId)
    {
      NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") moving from state " << *m_state << " to state " << *state << ".");

      m_state->Unselected (*this);
      SetState (state);
      state->Selected (*this);

      m_stateChangeTrace (GetTxSsrc (), GetCallInfo ()->GetCallId (), GetInstanceTypeId ().GetName (), currStateId.GetName (), stateId.GetName ());
    }
}


McpttFloorMsgFieldIndic
McpttOnNetworkFloorArbitrator::GetIndicator (void) const
{
  McpttFloorMsgFieldIndic indicator;
  uint8_t callTypeId = GetCallInfo ()->GetCallTypeId ();

  if (IsDualFloor ())
    {
      indicator.Indicate (McpttFloorMsgFieldIndic::DUAL_FLOOR);
    }

  if (callTypeId == McpttCallMsgFieldCallType::BASIC_GROUP)
    {
      indicator.Indicate (McpttFloorMsgFieldIndic::NORMAL_CALL);
    }
  else if (callTypeId == McpttCallMsgFieldCallType::BROADCAST_GROUP)
    {
      indicator.Indicate (McpttFloorMsgFieldIndic::BROADCAST_CALL);
    }
  else if (callTypeId == McpttCallMsgFieldCallType::EMERGENCY_GROUP)
    {
      indicator.Indicate (McpttFloorMsgFieldIndic::EMERGENCY_CALL);
    }
  else if (callTypeId == McpttCallMsgFieldCallType::IMMINENT_PERIL_GROUP)
    {
      indicator.Indicate (McpttFloorMsgFieldIndic::IMMINENT_CALL);
    }
  else if (callTypeId == McpttCallMsgFieldCallType::PRIVATE)
    {
      indicator.Indicate (McpttFloorMsgFieldIndic::NORMAL_CALL);
    }
  else if (callTypeId == McpttCallMsgFieldCallType::EMERGENCY_PRIVATE)
    {
      indicator.Indicate (McpttFloorMsgFieldIndic::EMERGENCY_CALL);
    }

  return indicator;
}

TypeId
McpttOnNetworkFloorArbitrator::GetInstanceTypeId (void) const
{
  return McpttOnNetworkFloorArbitrator::GetTypeId ();
}

uint32_t
McpttOnNetworkFloorArbitrator::GetNParticipants (void) const
{
  return m_participants.size ();
}

Ptr<McpttOnNetworkFloorTowardsParticipant>
McpttOnNetworkFloorArbitrator::GetParticipant (const uint32_t ssrc) const
{
  NS_LOG_FUNCTION (this);

  Ptr<McpttOnNetworkFloorTowardsParticipant> participant = 0;
  std::vector<Ptr<McpttOnNetworkFloorTowardsParticipant> >::const_iterator it = m_participants.begin ();

  while (participant == 0 && it != m_participants.end ())
    {
      if ((*it)->GetStoredSsrc () == ssrc)
        {
          participant = *it;
        }

      it++;
    }

  return participant;
}

McpttEntityId
McpttOnNetworkFloorArbitrator::GetStateId (void) const
{
  McpttEntityId stateId = m_state->GetInstanceStateId ();

  return stateId;
}

void
McpttOnNetworkFloorArbitrator::ImplicitFloorRequest (McpttOnNetworkFloorTowardsParticipant& participant)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << ": McpttOnNetworkFloorArbitrator (" << this << ") taking implicit floor request.");

  m_state->ImplicitFloorRequest (*this, participant);
}

bool
McpttOnNetworkFloorArbitrator::IsAudioCutIn (void) const
{
  return m_audioCutIn;
}


bool
McpttOnNetworkFloorArbitrator::IsAckRequired (void) const
{
  return m_ackRequired;
}

bool
McpttOnNetworkFloorArbitrator::IsDualFloor (void) const
{
  return (GetDualControl ()->IsStarted ());
}

bool
McpttOnNetworkFloorArbitrator::IsDualFloorSupported (void) const
{
  return m_dualFloorSupported;
}

bool
McpttOnNetworkFloorArbitrator::IsFloorOccupied (void) const
{
  return m_state->IsFloorOccupied (*this);
}

bool
McpttOnNetworkFloorArbitrator::IsMcGranted (void) const
{
  return m_mcGranted;
}

bool
McpttOnNetworkFloorArbitrator::IsPreemptive (const McpttFloorMsgRequest& msg) const
{
  bool preemptive = false;

  //if (!GetParticipant (msg.GetSsrc ())->IsReceiveOnly ())
    //{
      if (msg.GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::NORMAL_CALL))
        {
          if (GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::NORMAL_CALL))
            {
              if (msg.GetPriority ().GetPriority () > GetStoredPriority ())
                {
                  preemptive = true;
                }
            }
          else if (GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::IMMINENT_CALL)
                  || GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::EMERGENCY_CALL))
            {
              preemptive = false;
            }
        }
      else if (msg.GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::IMMINENT_CALL))
        {
          if (GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::NORMAL_CALL))
            {
              preemptive = true;
            }
          else if (GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::IMMINENT_CALL))
            {
              if (msg.GetPriority ().GetPriority () > GetStoredPriority ())
                {
                  preemptive = true;
                }
            }
          else if (GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::EMERGENCY_CALL))
            {
              preemptive = false;
            }
        }
      else if (msg.GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::EMERGENCY_CALL))
        {
          if (GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::NORMAL_CALL)
              || GetIndicator ().IsIndicated (McpttFloorMsgFieldIndic::IMMINENT_CALL))
            {
              preemptive = true;
            }
          else
            {
              if (msg.GetPriority ().GetPriority () > GetStoredPriority ())
                {
                  preemptive = true;
                }
            }
        }
      else
        {
          NS_FATAL_ERROR ("No call type indicated.");
        }
    //}

  return preemptive;
}

bool
McpttOnNetworkFloorArbitrator::IsStarted (void) const
{
  return GetStateId () != McpttOnNetworkFloorArbitratorStateStartStop::GetStateId ();
}

uint16_t
McpttOnNetworkFloorArbitrator::NextSeqNum (void)
{
  NS_LOG_FUNCTION (this);

  m_seqNum++;

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") next sequence number = " << (uint32_t)m_seqNum << ".");

  return m_seqNum;
}

void
McpttOnNetworkFloorArbitrator::ReceiveFloorRelease (const McpttFloorMsgRelease& msg)
{
  NS_LOG_FUNCTION (this << msg);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") received " << msg.GetInstanceTypeId () << ".");

  m_state->ReceiveFloorRelease (*this, msg);

  if (!m_rxCb.IsNull ())
    {
      m_rxCb (msg);
    }
}

void
McpttOnNetworkFloorArbitrator::ReceiveFloorRequest (const McpttFloorMsgRequest& msg)
{
  NS_LOG_FUNCTION (this << msg);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") received " << msg.GetInstanceTypeId () << ".");

  m_state->ReceiveFloorRequest (*this, msg);

  if (!m_rxCb.IsNull ())
    {
      m_rxCb (msg);
    }
}

void
McpttOnNetworkFloorArbitrator::ReceiveMedia (const McpttMediaMsg& msg)
{
  NS_LOG_FUNCTION (this << msg);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") received " << msg.GetInstanceTypeId () << ".");

  m_state->ReceiveMedia (*this, msg);
}

void
McpttOnNetworkFloorArbitrator::SendTo (McpttFloorMsg& msg, const uint32_t ssrc)
{
  NS_LOG_FUNCTION (this << msg);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") sending " << msg << " to " << ssrc << ".");

  bool found = false;
  std::vector<Ptr<McpttOnNetworkFloorTowardsParticipant> >::iterator it = m_participants.begin ();

  while (it != m_participants.end () && !found)
    {
      if ((*it)->GetStoredSsrc () == ssrc)
        {
          found = true;
          (*it)->Send (msg);
          if (!m_txCb.IsNull ())
            {
              m_txCb (msg);
            }
        }
      it++;
    }
}

void
McpttOnNetworkFloorArbitrator::SendToAll (McpttFloorMsg& msg)
{
  NS_LOG_FUNCTION (this << msg);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") sending " << msg << " to all.");

  std::vector<Ptr<McpttOnNetworkFloorTowardsParticipant> >::iterator it = m_participants.begin ();

  while (it != m_participants.end ())
    {
      (*it)->Send (msg);
      it++;
      if (!m_txCb.IsNull ())
        {
          m_txCb (msg);
        }
    }
}

void
McpttOnNetworkFloorArbitrator::SendToAllExcept (McpttFloorMsg& msg, const uint32_t ssrc)
{
  NS_LOG_FUNCTION (this << msg);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << "s: McpttOnNetworkFloorArbitrator (" << this << ") sending " << msg << " to all except " << ssrc << ".");

  std::vector<Ptr<McpttOnNetworkFloorTowardsParticipant> >::iterator it = m_participants.begin ();

  while (it != m_participants.end ())
    {
      if ((*it)->GetStoredSsrc () != ssrc)
        {
          (*it)->Send (msg);

          if (!m_txCb.IsNull ())
            {
              m_txCb (msg);
            }
        }
      it++;
    }
}

void
McpttOnNetworkFloorArbitrator::SetDelayT1 (const Time& delayT1)
{
  NS_LOG_FUNCTION (this << delayT1);

  GetT1 ()->SetDelay (delayT1);
}

void
McpttOnNetworkFloorArbitrator::SetDelayT2 (const Time& delayT2)
{
  NS_LOG_FUNCTION (this << delayT2);

  GetT2 ()->SetDelay (delayT2);
}

void
McpttOnNetworkFloorArbitrator::SetDelayT3 (const Time& delayT3)
{
  NS_LOG_FUNCTION (this << delayT3);

  GetT3 ()->SetDelay (delayT3);
}

void
McpttOnNetworkFloorArbitrator::SetDelayT4 (const Time& delayT4)
{
  NS_LOG_FUNCTION (this << delayT4);

  GetT4 ()->SetDelay (delayT4);
}

void
McpttOnNetworkFloorArbitrator::SetDelayT7 (const Time& delayT7)
{
  NS_LOG_FUNCTION (this << delayT7);

  GetT7 ()->SetDelay (delayT7);
}

void
McpttOnNetworkFloorArbitrator::SetDelayT20 (const Time& delayT20)
{
  NS_LOG_FUNCTION (this << delayT20);

  GetT20 ()->SetDelay (delayT20);
}

void
McpttOnNetworkFloorArbitrator::SetLimitC7 (uint32_t limitC7)
{
  NS_LOG_FUNCTION (this << limitC7);

  GetC7 ()->SetLimit (limitC7);
}

void
McpttOnNetworkFloorArbitrator::SetLimitC20 (uint32_t limitC20)
{
  NS_LOG_FUNCTION (this << limitC20);

  GetC20 ()->SetLimit (limitC20);
}

void
McpttOnNetworkFloorArbitrator::Start (void)
{
  NS_LOG_FUNCTION (this);

  for (std::vector<Ptr<McpttOnNetworkFloorTowardsParticipant> >::iterator it = m_participants.begin (); it != m_participants.end (); it++)
    {
      (*it)->Start ();
    }
}

void
McpttOnNetworkFloorArbitrator::Stop (void)
{
  NS_LOG_FUNCTION (this);

  for (std::vector<Ptr<McpttOnNetworkFloorTowardsParticipant> >::iterator it = m_participants.begin (); it != m_participants.end (); it++)
    {
      (*it)->Stop ();
    }

  if (GetT1 ()->IsRunning ())
    {
      GetT1 ()->Stop ();
    }

  if (GetT2 ()->IsRunning ())
    {
      GetT2 ()->Stop ();
    }

  if (GetT3 ()->IsRunning ())
    {
      GetT3 ()->Stop ();
    }

  if (GetT4 ()->IsRunning ())
    {
      GetT4 ()->Stop ();
    }

  if (GetT7 ()->IsRunning ())
    {
      GetT7 ()->Stop ();
    }

  if (GetT20 ()->IsRunning ())
    {
      GetT20 ()->Stop ();
    }

}

void
McpttOnNetworkFloorArbitrator::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_c7 = 0;
  m_c20 = 0;
  m_owner = 0;
  m_state = 0;
  m_t1 = 0;
  m_t2 = 0;
  m_t3 = 0;
  m_t4 = 0;
  m_t7 = 0;
  m_t20 = 0;
}

void
McpttOnNetworkFloorArbitrator::ExpiryOfT1 (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << ": McpttOnNetworkFloorArbitrator T1 expired.");

  m_state->ExpiryOfT1 (*this);
}

void
McpttOnNetworkFloorArbitrator::ExpiryOfT2 (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << ": McpttOnNetworkFloorArbitrator T2 expired.");

  m_state->ExpiryOfT2 (*this);
}

void
McpttOnNetworkFloorArbitrator::ExpiryOfT3 (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << ": McpttOnNetworkFloorArbitrator T3 expired.");

  m_state->ExpiryOfT3 (*this);
}

void
McpttOnNetworkFloorArbitrator::ExpiryOfT4 (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << ": McpttOnNetworkFloorArbitrator T4 expired.");

  m_state->ExpiryOfT4 (*this);
}

void
McpttOnNetworkFloorArbitrator::ExpiryOfT7 (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << ": McpttOnNetworkFloorArbitrator T7 expired.");

  m_state->ExpiryOfT7 (*this);
}

void
McpttOnNetworkFloorArbitrator::ExpiryOfT20 (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC (Simulator::Now ().GetSeconds () << ": McpttOnNetworkFloorArbitrator T20 expired.");

  m_state->ExpiryOfT20 (*this);
}

Ptr<McpttCallControlInfo>
McpttOnNetworkFloorArbitrator::GetCallInfo (void) const
{
  NS_LOG_FUNCTION (this);

  return m_callInfo;
}

Ptr<McpttCounter>
McpttOnNetworkFloorArbitrator::GetC7 (void) const
{
  NS_LOG_FUNCTION (this);

  return m_c7;
}

Ptr<McpttCounter>
McpttOnNetworkFloorArbitrator::GetC20 (void) const
{
  NS_LOG_FUNCTION (this);

  return m_c20;
}

Ptr<McpttOnNetworkFloorDualControl>
McpttOnNetworkFloorArbitrator::GetDualControl (void) const
{
  NS_LOG_FUNCTION (this);

  return m_dualControl;
}

McpttOnNetworkFloorServerApp*
McpttOnNetworkFloorArbitrator::GetOwner (void) const
{
  NS_LOG_FUNCTION (this);

  return m_owner;
}

uint8_t
McpttOnNetworkFloorArbitrator::GetStoredPriority (void) const
{
  NS_LOG_FUNCTION (this);

  return m_storedPriority;
}

Ptr<McpttFloorQueue>
McpttOnNetworkFloorArbitrator::GetQueue (void) const
{
  NS_LOG_FUNCTION (this);

  return m_queue;
}

uint16_t
McpttOnNetworkFloorArbitrator::GetRejectCause (void) const
{
  NS_LOG_FUNCTION (this);

  return m_rejectCause;
}

uint32_t
McpttOnNetworkFloorArbitrator::GetStoredSsrc (void) const
{
  NS_LOG_FUNCTION (this);

  return m_storedSsrc;
}

McpttFloorMsgFieldTrackInfo
McpttOnNetworkFloorArbitrator::GetTrackInfo (void) const
{
  NS_LOG_FUNCTION (this);

  return m_trackInfo;
}

uint32_t
McpttOnNetworkFloorArbitrator::GetTxSsrc (void) const
{
  NS_LOG_FUNCTION (this);

  return m_txSsrc;
}

Ptr<McpttTimer>
McpttOnNetworkFloorArbitrator::GetT1 (void) const
{
  NS_LOG_FUNCTION (this);

  return m_t1;
}

Ptr<McpttTimer>
McpttOnNetworkFloorArbitrator::GetT2 (void) const
{
  NS_LOG_FUNCTION (this);

  return m_t2;
}

Ptr<McpttTimer>
McpttOnNetworkFloorArbitrator::GetT3 (void) const
{
  NS_LOG_FUNCTION (this);

  return m_t3;
}

Ptr<McpttTimer>
McpttOnNetworkFloorArbitrator::GetT4 (void) const
{
  NS_LOG_FUNCTION (this);

  return m_t4;
}

Ptr<McpttTimer>
McpttOnNetworkFloorArbitrator::GetT7 (void) const
{
  NS_LOG_FUNCTION (this);

  return m_t7;
}

Ptr<McpttTimer>
McpttOnNetworkFloorArbitrator::GetT20 (void) const
{
  NS_LOG_FUNCTION (this);

  return m_t20;
}

void
McpttOnNetworkFloorArbitrator::SetCallInfo (const Ptr<McpttCallControlInfo> callInfo)
{
  NS_LOG_FUNCTION (this);

  m_callInfo = callInfo;
}

void
McpttOnNetworkFloorArbitrator::SetDualControl (const Ptr<McpttOnNetworkFloorDualControl> dualControl)
{
  NS_LOG_FUNCTION (this);

  m_dualControl = dualControl;
}

void
McpttOnNetworkFloorArbitrator::SetOwner (McpttOnNetworkFloorServerApp* const& owner)
{
  NS_LOG_FUNCTION (this);

  m_owner = owner;
}

void
McpttOnNetworkFloorArbitrator::SetRejectCause (const uint16_t rejectCause)
{
  NS_LOG_FUNCTION (this);

  m_rejectCause = rejectCause;
}

void
McpttOnNetworkFloorArbitrator::SetRxCb (const Callback<void, const McpttFloorMsg&>  rxCb)
{
  NS_LOG_FUNCTION (this);

  m_rxCb = rxCb;
}

void
McpttOnNetworkFloorArbitrator::SetState (Ptr<McpttOnNetworkFloorArbitratorState>  state)
{
  NS_LOG_FUNCTION (this << state);

  m_state = state;
}

void
McpttOnNetworkFloorArbitrator::SetStateChangeCb (const Callback<void, const McpttEntityId&, const McpttEntityId&>  stateChangeCb)
{
  NS_LOG_FUNCTION (this);

  m_stateChangeCb = stateChangeCb;
}

void
McpttOnNetworkFloorArbitrator::SetStoredSsrc (const uint32_t storedSsrc)
{
  NS_LOG_FUNCTION (this << storedSsrc);
  
  m_storedSsrc = storedSsrc;
}

void
McpttOnNetworkFloorArbitrator::SetStoredPriority (uint8_t storedPriority)
{
  NS_LOG_FUNCTION (this << +storedPriority);

  m_storedPriority = storedPriority;
}

void
McpttOnNetworkFloorArbitrator::SetTrackInfo (const McpttFloorMsgFieldTrackInfo& trackInfo)
{
  NS_LOG_FUNCTION (this);

  m_trackInfo = trackInfo;
}

void
McpttOnNetworkFloorArbitrator::SetTxCb (const Callback<void, const McpttFloorMsg&>  txCb)
{
  NS_LOG_FUNCTION (this);

  m_txCb = txCb;
}
/** McpttOnNetworkFloorArbitrator - end **/

} // namespace ns3
