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

#include <ns3/log.h>
#include <ns3/object.h>
#include <ns3/type-id.h>
#include <ns3/sip-header.h>

#include "mcptt-on-network-call-machine-client.h"
#include "mcptt-call-msg.h"
#include "mcptt-on-network-floor-participant.h"
#include "mcptt-ptt-app.h"
#include "mcptt-sdp-fmtp-header.h"

#include "mcptt-on-network-call-machine-client-state.h"

namespace ns3 {

/** McpttOnNetworkCallMachineClientState - begin **/
NS_LOG_COMPONENT_DEFINE ("McpttOnNetworkCallMachineClientState");

McpttOnNetworkCallMachineClientState::McpttOnNetworkCallMachineClientState (void)
  : SimpleRefCount<McpttOnNetworkCallMachineClientState> ()
{
  NS_LOG_FUNCTION (this);
}

McpttOnNetworkCallMachineClientState::~McpttOnNetworkCallMachineClientState (void)
{
  NS_LOG_FUNCTION (this);
}

McpttEntityId
McpttOnNetworkCallMachineClientState::GetInstanceStateId (void) const
{
  return McpttEntityId ();
}

bool
McpttOnNetworkCallMachineClientState::IsCallOngoing (const McpttOnNetworkCallMachineClient& machine) const
{
  return false;
}

void
McpttOnNetworkCallMachineClientState::ReceiveInvite (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine << from << pkt);
  NS_LOG_LOGIC ("Ignoring INVITE");
}

void
McpttOnNetworkCallMachineClientState::ReceiveBye (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine << from << pkt);
  NS_LOG_LOGIC ("Ignoring BYE"); 
}

void
McpttOnNetworkCallMachineClientState::ReceiveResponse (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine << from << pkt);
  NS_LOG_LOGIC ("Ignoring response"); 
}

void
McpttOnNetworkCallMachineClientState::InitiateCall (McpttOnNetworkCallMachineClient& machine)
{
  NS_LOG_FUNCTION (this << &machine);
  NS_LOG_LOGIC ("Ignoring initiate call."); 
}

void
McpttOnNetworkCallMachineClientState::ReleaseCall (McpttOnNetworkCallMachineClient& machine)
{
  NS_LOG_FUNCTION (this << &machine);

  NS_LOG_LOGIC ("Ignoring release call."); 
}

/** McpttOnNetworkCallMachineClientState - end **/

std::ostream&
operator<< (std::ostream& os, const McpttOnNetworkCallMachineClientState& inst)
{
  McpttEntityId stateId = inst.GetInstanceStateId ();

  os << stateId;

  return os;
}

/** McpttOnNetworkCallMachineClientStateS1 - begin **/
Ptr<McpttOnNetworkCallMachineClientStateS1>
McpttOnNetworkCallMachineClientStateS1::GetInstance (void)
{
  static Ptr<McpttOnNetworkCallMachineClientStateS1> instance = Create<McpttOnNetworkCallMachineClientStateS1> ();
  return instance;
}

McpttEntityId
McpttOnNetworkCallMachineClientStateS1::GetStateId (void)
{
  static McpttEntityId stateId = McpttEntityId (1, "'S1: start-stop'");
  return stateId;
}

McpttOnNetworkCallMachineClientStateS1::McpttOnNetworkCallMachineClientStateS1 (void)
  : McpttOnNetworkCallMachineClientState ()
{
  NS_LOG_FUNCTION (this);
}

McpttOnNetworkCallMachineClientStateS1::~McpttOnNetworkCallMachineClientStateS1 (void)
{
  NS_LOG_FUNCTION (this);
}

McpttEntityId
McpttOnNetworkCallMachineClientStateS1::GetInstanceStateId (void) const
{
  return McpttOnNetworkCallMachineClientStateS1::GetStateId ();
}

void
McpttOnNetworkCallMachineClientStateS1::InitiateCall (McpttOnNetworkCallMachineClient& machine)
{
  NS_LOG_FUNCTION (this << &machine);

  Ptr<McpttOnNetworkFloorParticipant> floorMachine = machine.GetOwner ()->GetFloorMachine ()->GetObject<McpttOnNetworkFloorParticipant> ();
  floorMachine->CallInitiated ();

  Ptr<Packet> pkt = Create<Packet> ();
  McpttSdpFmtpHeader fmtpHeader;
  fmtpHeader.SetMcGranted (true); // No attribute for this; always enabled
  fmtpHeader.SetMcPriority (floorMachine->GetPriority ());
  fmtpHeader.SetMcQueueing (true); // No attribute for this; always enabled
  fmtpHeader.SetMcImplicitRequest (floorMachine->IsImplicitRequest ());
  pkt->AddHeader (fmtpHeader);
  SipHeader sipHeader;
  sipHeader.SetMessageType (SipHeader::SIP_REQUEST);
  sipHeader.SetMethod (SipHeader::INVITE);
  sipHeader.SetRequestUri (machine.GetGrpId ().GetGrpId ());
  sipHeader.SetFrom (machine.GetOwner ()->GetOwner ()->GetUserId ());
  sipHeader.SetTo (machine.GetGrpId ().GetGrpId ());
  sipHeader.SetCallId (machine.GetOwner ()->GetCallId ());
  pkt->AddHeader (sipHeader);
  machine.SendCallControlPacket (pkt);

  machine.SetState (McpttOnNetworkCallMachineClientStateS2::GetInstance ());
}

void
McpttOnNetworkCallMachineClientStateS1::ReceiveInvite (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine << from << pkt);

  SipHeader receivedSipHeader;
  pkt->RemoveHeader (receivedSipHeader);
  McpttSdpFmtpHeader sdpHeader;
  pkt->RemoveHeader (sdpHeader);

  Ptr<McpttOnNetworkFloorParticipant> floorMachine = machine.GetOwner ()->GetFloorMachine ()->GetObject<McpttOnNetworkFloorParticipant> ();
  floorMachine->CallEstablished (sdpHeader.GetMcGranted (), sdpHeader.GetMcPriority ());

  // Notify McpttPttApp of session initiation
  machine.GetOwner ()->GetOwner ()->SessionInitiateRequest ();

  SipHeader sipHeader;
  sipHeader.SetMessageType (SipHeader::SIP_RESPONSE);
  sipHeader.SetStatusCode (200);
  sipHeader.SetFrom (machine.GetOwner ()->GetOwner ()->GetUserId ());
  sipHeader.SetTo (machine.GetGrpId ().GetGrpId ());
  sipHeader.SetCallId (machine.GetOwner ()->GetCallId ());
  Ptr<Packet> response = Create<Packet> ();
  response->AddHeader (sipHeader);
  machine.SendCallControlPacket (response);

  machine.SetState (McpttOnNetworkCallMachineClientStateS3::GetInstance ());
}

/** McpttOnNetworkCallMachineClientStateS2 - begin **/
Ptr<McpttOnNetworkCallMachineClientStateS2>
McpttOnNetworkCallMachineClientStateS2::GetInstance (void)
{
  static Ptr<McpttOnNetworkCallMachineClientStateS2> instance = Create<McpttOnNetworkCallMachineClientStateS2> ();
  return instance;
}

McpttEntityId
McpttOnNetworkCallMachineClientStateS2::GetStateId (void)
{
  static McpttEntityId stateId = McpttEntityId (2, "'S2: initiating'");
  return stateId;
}

McpttOnNetworkCallMachineClientStateS2::McpttOnNetworkCallMachineClientStateS2 (void)
  : McpttOnNetworkCallMachineClientState ()
{
  NS_LOG_FUNCTION (this);
}

McpttOnNetworkCallMachineClientStateS2::~McpttOnNetworkCallMachineClientStateS2 (void)
{
  NS_LOG_FUNCTION (this);
}

McpttEntityId
McpttOnNetworkCallMachineClientStateS2::GetInstanceStateId (void) const
{
  return McpttOnNetworkCallMachineClientStateS2::GetStateId ();
}

void
McpttOnNetworkCallMachineClientStateS2::ReceiveInvite (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine);
  // This indicates a setup collision; this client's INVITE has beaten
  // my INVITE to the server.  Handle this transaction as if my INVITE
  // transaction did not happen (i.e. cancel the initiating transaction
  // and handle this as if from state S1)
  NS_LOG_LOGIC ("Handle received INVITE despite being in state S2 (collision)");
  SipHeader receivedSipHeader;
  pkt->RemoveHeader (receivedSipHeader);
  McpttSdpFmtpHeader sdpHeader;
  pkt->RemoveHeader (sdpHeader);
  Ptr<McpttOnNetworkFloorParticipant> floorMachine = machine.GetOwner ()->GetFloorMachine ()->GetObject<McpttOnNetworkFloorParticipant> ();
  floorMachine->CallEstablished (sdpHeader.GetMcGranted (), sdpHeader.GetMcPriority ());

  // Notify McpttPttApp of session initiation
  machine.GetOwner ()->GetOwner ()->SessionInitiateRequest ();
  
  SipHeader sipHeader;
  sipHeader.SetMessageType (SipHeader::SIP_RESPONSE);
  sipHeader.SetStatusCode (200);
  sipHeader.SetFrom (machine.GetOwner ()->GetOwner ()->GetUserId ());
  sipHeader.SetTo (machine.GetGrpId ().GetGrpId ());
  sipHeader.SetCallId (machine.GetOwner ()->GetCallId ());
  Ptr<Packet> response = Create<Packet> ();
  response->AddHeader (sipHeader);
  machine.SendCallControlPacket (response);
  machine.SetState (McpttOnNetworkCallMachineClientStateS3::GetInstance ());
}

void
McpttOnNetworkCallMachineClientStateS2::ReceiveResponse (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine << from << pkt);

  SipHeader sipHeader;
  pkt->RemoveHeader (sipHeader);
  McpttSdpFmtpHeader sdpHeader;
  pkt->RemoveHeader (sdpHeader);

  Ptr<McpttOnNetworkFloorParticipant> floorMachine = machine.GetOwner ()->GetFloorMachine ()->GetObject<McpttOnNetworkFloorParticipant> ();
  floorMachine->CallEstablished (sdpHeader.GetMcGranted (), sdpHeader.GetMcPriority ());

  // Originating client is responsible for scheduling the release of the call
  NS_ABORT_MSG_UNLESS (machine.GetOwner ()->GetStopTime () >= Simulator::Now (), "Stop time in the past");
  Simulator::Schedule (machine.GetOwner ()->GetStopTime ()  - Simulator::Now (), &McpttPttApp::ReleaseCall, machine.GetOwner ()->GetOwner ());
  machine.SetState (McpttOnNetworkCallMachineClientStateS3::GetInstance ());
}

void
McpttOnNetworkCallMachineClientStateS2::ReceiveBye (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine << from << pkt);
  machine.SetState (McpttOnNetworkCallMachineClientStateS1::GetInstance ());
}

void
McpttOnNetworkCallMachineClientStateS2::ReleaseCall (McpttOnNetworkCallMachineClient& machine)
{
  NS_LOG_FUNCTION (this << &machine);
  machine.SetState (McpttOnNetworkCallMachineClientStateS4::GetInstance ());
}

/** McpttOnNetworkCallMachineClientStateS2 - end **/

/** McpttOnNetworkCallMachineClientStateS3 - begin **/
Ptr<McpttOnNetworkCallMachineClientStateS3>
McpttOnNetworkCallMachineClientStateS3::GetInstance (void)
{
  static Ptr<McpttOnNetworkCallMachineClientStateS3> instance = Create<McpttOnNetworkCallMachineClientStateS3> ();
  return instance;
}

McpttEntityId
McpttOnNetworkCallMachineClientStateS3::GetStateId (void)
{
  static McpttEntityId stateId = McpttEntityId (3, "'S3: part of ongoing call'");

  return stateId;
}

McpttOnNetworkCallMachineClientStateS3::McpttOnNetworkCallMachineClientStateS3 (void)
  : McpttOnNetworkCallMachineClientState ()
{
  NS_LOG_FUNCTION (this);
}

McpttOnNetworkCallMachineClientStateS3::~McpttOnNetworkCallMachineClientStateS3 (void)
{
  NS_LOG_FUNCTION (this);
}

McpttEntityId
McpttOnNetworkCallMachineClientStateS3::GetInstanceStateId (void) const
{
  return McpttOnNetworkCallMachineClientStateS3::GetStateId ();
}

bool
McpttOnNetworkCallMachineClientStateS3::IsCallOngoing (const McpttOnNetworkCallMachineClient& machine) const
{
  NS_LOG_FUNCTION (this << &machine);
  return true;
}

void
McpttOnNetworkCallMachineClientStateS3::ReceiveResponse (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine << from << pkt);
  NS_LOG_LOGIC ("Ignoring response in established state"); 
}

void
McpttOnNetworkCallMachineClientStateS3::ReceiveBye (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine << from << pkt);

  Ptr<McpttOnNetworkFloorParticipant> floorMachine = machine.GetOwner ()->GetFloorMachine ()->GetObject<McpttOnNetworkFloorParticipant> ();
  floorMachine->CallRelease1 ();
  floorMachine->CallRelease2 ();

  // Notify McpttPttApp of session release
  machine.GetOwner ()->GetOwner ()->SessionReleaseRequest ();

  SipHeader sipHeader;
  sipHeader.SetMessageType (SipHeader::SIP_RESPONSE);
  sipHeader.SetStatusCode (200);
  sipHeader.SetFrom (machine.GetOwner ()->GetOwner ()->GetUserId ());
  sipHeader.SetTo (machine.GetGrpId ().GetGrpId ());
  sipHeader.SetCallId (machine.GetOwner ()->GetCallId ());
  Ptr<Packet> response = Create<Packet> ();
  response->AddHeader (sipHeader);
  machine.SendCallControlPacket (response);

  machine.SetState (McpttOnNetworkCallMachineClientStateS1::GetInstance ());
}

void
McpttOnNetworkCallMachineClientStateS3::ReleaseCall (McpttOnNetworkCallMachineClient& machine)
{
  NS_LOG_FUNCTION (this << &machine);
  Ptr<McpttOnNetworkFloorParticipant> floorMachine = machine.GetOwner ()->GetFloorMachine ()->GetObject<McpttOnNetworkFloorParticipant> ();
  floorMachine->CallRelease1 ();

  SipHeader sipHeader;
  sipHeader.SetMessageType (SipHeader::SIP_REQUEST);
  sipHeader.SetMethod (SipHeader::BYE);
  sipHeader.SetRequestUri (machine.GetGrpId ().GetGrpId ());
  sipHeader.SetFrom (machine.GetOwner ()->GetOwner ()->GetUserId ());
  sipHeader.SetTo (machine.GetGrpId ().GetGrpId ());
  sipHeader.SetCallId (machine.GetOwner ()->GetCallId ());
  Ptr<Packet> pkt = Create<Packet> ();
  pkt->AddHeader (sipHeader);
  machine.SendCallControlPacket (pkt);

  machine.SetState (McpttOnNetworkCallMachineClientStateS4::GetInstance ());
}
/** McpttOnNetworkCallMachineClientStateS3 - end **/

/** McpttOnNetworkCallMachineClientStateS4 - begin **/
Ptr<McpttOnNetworkCallMachineClientStateS4>
McpttOnNetworkCallMachineClientStateS4::GetInstance (void)
{
  static Ptr<McpttOnNetworkCallMachineClientStateS4> instance = Create<McpttOnNetworkCallMachineClientStateS4> ();
  return instance;
}

McpttEntityId
McpttOnNetworkCallMachineClientStateS4::GetStateId (void)
{
  static McpttEntityId stateId = McpttEntityId (4, "'S4: releasing'");

  return stateId;
}

McpttOnNetworkCallMachineClientStateS4::McpttOnNetworkCallMachineClientStateS4 (void)
  : McpttOnNetworkCallMachineClientState ()
{
  NS_LOG_FUNCTION (this);
}

McpttOnNetworkCallMachineClientStateS4::~McpttOnNetworkCallMachineClientStateS4 (void)
{
  NS_LOG_FUNCTION (this);
}

McpttEntityId
McpttOnNetworkCallMachineClientStateS4::GetInstanceStateId (void) const
{
  return McpttOnNetworkCallMachineClientStateS4::GetStateId ();
}
void
McpttOnNetworkCallMachineClientStateS4::ReceiveBye (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  // colliding BYEs
  NS_LOG_FUNCTION (this << &machine << from << pkt);
  machine.SetState (McpttOnNetworkCallMachineClientStateS1::GetInstance ());
}

void
McpttOnNetworkCallMachineClientStateS4::ReceiveResponse (McpttOnNetworkCallMachineClient& machine, uint32_t from, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << &machine << from << pkt);
  Ptr<McpttOnNetworkFloorParticipant> floorMachine = machine.GetOwner ()->GetFloorMachine ()->GetObject<McpttOnNetworkFloorParticipant> ();
  floorMachine->CallRelease2 ();
  machine.SetState (McpttOnNetworkCallMachineClientStateS1::GetInstance ());
}

/** McpttOnNetworkCallMachineClientStateS4 - end **/

} // namespace ns3
