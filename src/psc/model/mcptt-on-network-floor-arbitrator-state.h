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

#ifndef MCPTT_ON_NETWORK_FLOOR_ARBITRATOR_STATE_H
#define MCPTT_ON_NETWORK_FLOOR_ARBITRATOR_STATE_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/type-id.h>

#include "mcptt-entity-id.h"
#include "mcptt-floor-msg.h"
#include "mcptt-floor-queue.h"
#include "mcptt-media-msg.h"

namespace ns3 {

class McpttOnNetworkFloorArbitrator;
class McpttOnNetworkFloorTowardsParticipant;

/**
 * \ingroup mcptt
 *
 * This class represents a floor control state machine state as described in TS
 * 24.380 version 14.8.0.
 *
 * 'Start-stop'
 *  - The floor control server is in this state when a new instance of the
 *    'general floor control operation' state machine has been intitated, or
 *    when a call has been released.
 *
 * 'G: Floor Idle'
 *  - The floor control server is in this state when no MCPTT user currently
 *    has permission to send media.
 *
 * 'G: Floor Taken'
 *  - The floor controler server is in this state when it has permitted one of
 *    the MCPTT clients in the MCPTT call to send media.
 *
 * 'G: Pending Floor Revoke'
 *  - The floor control server is in this state after having sent a Floor
 *    Revoke message to the permitted floor participant.
 *
 * 'Releasing'
 *  - The floor control server is in this state while waiting for the
 *    application and signallying plane to finalize the disconnection of an
 *    MCPTT call.
 *
 * 'G: Floor Initialising'
 *  - The floor control server is in this state while waiting for all invited
 *    constituent MCPTT groups to reply with a final SIP response.
 */
class McpttOnNetworkFloorArbitratorState : public SimpleRefCount<McpttOnNetworkFloorArbitratorState>
{
public:
 /**
  * \brief The destructor of the McpttOnNetworkFloorArbitratorState class.
  */
 virtual ~McpttOnNetworkFloorArbitratorState (void);
 /**
  * Notifies the floor machine state that the call was initialized.
  * \param machine The FSM.
  * \param participant The participant that initiated the call.
  */
 virtual void CallInitialized (McpttOnNetworkFloorArbitrator& machine, McpttOnNetworkFloorTowardsParticipant& participant) const;
 /**
  * Indicates to the floor control server that the call has been released (part I).
  * \param machine The FSM.
  */
 virtual void CallRelease1 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Indicates to the floor control server that the call has been released (part II).
  * \param machine The FSM.
  */
 virtual void CallRelease2 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Indicates to the floor control server that the client has disconnected.
  * \param machine The FSM.
  */
 virtual void ClientRelease (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Enters the given state.
  * \param machine The FSM.
  */
 virtual void Enter (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T1 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT1 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T2 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT2 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T3 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT3 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T4 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT4 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T7 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT7 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T20 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT20 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Gets the ID of the machine state.
  * \returns The ID of the state.
  */
 virtual McpttEntityId GetInstanceStateId (void) const;
 /**
  * Notifies this machine state that an implicit floor request has been received.
  * \param machine The FSM.
  * \param participant The participant that initiated the implicit floor request.
  */
 virtual void ImplicitFloorRequest (McpttOnNetworkFloorArbitrator& machine, McpttOnNetworkFloorTowardsParticipant& participant) const;
 /**
  * Indicates if a client is permitted to send media.
  * \returns True, if a client is permited; owtherwise, false.
  */
 virtual bool IsFloorOccupied (const McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Receives a floor release message.
  * \param machine The FSM.
  * \param msg The received message.
  */
 virtual void ReceiveFloorRelease (McpttOnNetworkFloorArbitrator& machine, const McpttFloorMsgRelease& msg) const;
 /**
  * Receives a floor request message.
  * \param machine The FSM.
  * \param msg The received message.
  */
 virtual void ReceiveFloorRequest (McpttOnNetworkFloorArbitrator& machine, const McpttFloorMsgRequest& msg) const;
 /**
  * Receives a media message.
  * \param machine The FSM.
  * \param msg The received message.
  */
 virtual void ReceiveMedia (McpttOnNetworkFloorArbitrator& machine, const McpttMediaMsg& msg) const;
 /**
  * Notifies the machine state that it has been selected.
  * \param machine The FSM.
  */
 virtual void Selected (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the machine state that it has been unselected.
  * \param machine The FSM.
  */
 virtual void Unselected (McpttOnNetworkFloorArbitrator& machine) const;
protected:
 /**
  * Creates an instance of the McpttOnNetworkFloorArbitratorState class.
  */
 McpttOnNetworkFloorArbitratorState (void);
};
/**
 * Overloaded output operator for the ns3::McpttOnNetworkFloorArbitratorState class.
 * \param os The output stream to write to.
 * \param state The state to write to the output stream.
 * \returns The output stream that was written to.
 */
std::ostream& operator<< (std::ostream& os, const ns3::McpttOnNetworkFloorArbitratorState& state);
/**
 * \ingroup mcptt
 * \brief A class that represents the 'Start-stop' state.
 */
class McpttOnNetworkFloorArbitratorStateStartStop : public McpttOnNetworkFloorArbitratorState
{
public:
 /**
  * Gets an instance of the McpttOnNetworkFloorArbitratorStateStartStop.
  * \returns An instance.
  */
 static Ptr<McpttOnNetworkFloorArbitratorStateStartStop> GetInstance (void);
 /**
  * \brief Gets the ID of the state.
  * \returns The ID of the state.
  */
 static McpttEntityId GetStateId (void);
 /**
  * \brief Creates an instance of the McpttOnNetworkFloorArbitratorStateStartStop class.
  */
 McpttOnNetworkFloorArbitratorStateStartStop (void);
 /**
  * \brief The destructor of the McpttOnNetworkFloorArbitratorStateStartStop class.
  */
 virtual ~McpttOnNetworkFloorArbitratorStateStartStop (void);
 /**
  * Gets the ID of the machine state.
  * \returns The ID of the state.
  */
 virtual McpttEntityId GetInstanceStateId (void) const;
 /**
  * Notifies the floor machine state that the call was initialized.
  * \param machine The FSM.
  * \param participant The participant that initiated the call.
  */
 virtual void CallInitialized (McpttOnNetworkFloorArbitrator& machine, McpttOnNetworkFloorTowardsParticipant& participant) const;
};
/**
 * \ingroup mcptt
 * \brief A class that represents the 'G: Floor Idle' state.
 */
class McpttOnNetworkFloorArbitratorStateIdle : public McpttOnNetworkFloorArbitratorState
{
public:
 /**
  * Gets an instance of the McpttOnNetworkFloorArbitratorStateIdle.
  * \returns An instance.
  */
 static Ptr<McpttOnNetworkFloorArbitratorStateIdle> GetInstance (void);
 /**
  * \brief Gets the ID of the state.
  * \returns The ID of the state.
  */
 static McpttEntityId GetStateId (void);
 /**
  * \brief Creates an instance of the McpttOnNetworkFloorArbitratorStateIdle class.
  */
 McpttOnNetworkFloorArbitratorStateIdle (void);
 /**
  * \brief The destructor of the McpttOnNetworkFloorArbitratorStateIdle class.
  */
 virtual ~McpttOnNetworkFloorArbitratorStateIdle (void);
 /**
  * Gets the ID of the machine state.
  * \returns The ID of the state.
  */
 virtual McpttEntityId GetInstanceStateId (void) const;
 /**
  * Enters this state.
  * \param machine The FSM.
  */
 virtual void Enter (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Receives a floor request message.
  * \param machine The FSM.
  * \param msg The received message.
  */
 virtual void ReceiveFloorRequest (McpttOnNetworkFloorArbitrator& machine, const McpttFloorMsgRequest& msg) const;
 /**
  * Notifies the floor machine state that timer T7 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT7 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T4 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT4 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies this machine state that an implicit floor request has been received.
  * \param machine The FSM.
  * \param participant The participant that initiated the implicit floor request.
  */
 virtual void ImplicitFloorRequest (McpttOnNetworkFloorArbitrator& machine, McpttOnNetworkFloorTowardsParticipant& participant) const;
};
/**
 * \ingroup mcptt
 * \brief A class that represents the 'G: Floor Taken' state.
 */
class McpttOnNetworkFloorArbitratorStateTaken : public McpttOnNetworkFloorArbitratorState
{
public:
 /**
  * Gets an instance of the McpttOnNetworkFloorArbitratorStateTaken.
  * \returns An instance.
  */
 static Ptr<McpttOnNetworkFloorArbitratorStateTaken> GetInstance (void);
 /**
  * \brief Gets the ID of the state.
  * \returns The ID of the state.
  */
 static McpttEntityId GetStateId (void);
 /**
  * \brief Creates an instance of the McpttOnNetworkFloorArbitratorStateTaken class.
  */
 McpttOnNetworkFloorArbitratorStateTaken (void);
 /**
  * \brief The destructor of the McpttOnNetworkFloorArbitratorStateTaken class.
  */
 virtual ~McpttOnNetworkFloorArbitratorStateTaken (void);
 /**
  * Gets the ID of the machine state.
  * \returns The ID of the state.
  */
 virtual McpttEntityId GetInstanceStateId (void) const;
 /**
  * Indicates if a client is permitted to send media.
  * \returns True, if a client is permited; owtherwise, false.
  */
 virtual bool IsFloorOccupied (const McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the machine state that it has been selected.
  * \param machine The FSM.
  */
 virtual void Enter (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T1 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT1 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T2 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT2 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Receives a media message.
  * \param machine The FSM.
  * \param msg The received message.
  */
 virtual void ReceiveMedia (McpttOnNetworkFloorArbitrator& machine, const McpttMediaMsg& msg) const;
 /**
  * Receives a floor release message.
  * \param machine The FSM.
  * \param msg The received message.
  */
 virtual void ReceiveFloorRelease (McpttOnNetworkFloorArbitrator& machine, const McpttFloorMsgRelease& msg) const;
 /**
  * Receives a floor request message.
  * \param machine The FSM.
  * \param msg The received message.
  */
 virtual void ReceiveFloorRequest (McpttOnNetworkFloorArbitrator& machine, const McpttFloorMsgRequest& msg) const;
 /**
  * Notifies the floor machine state that timer T20 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT20 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Indicates to the floor control server that the client has disconnected.
  * \param machine The FSM.
  */
 virtual void ClientRelease (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies this machine state that an implicit floor request has been received.
  * \param machine The FSM.
  * \param participant The participant that initiated the implicit floor request.
  */
 virtual void ImplicitFloorRequest (McpttOnNetworkFloorArbitrator& machine, McpttOnNetworkFloorTowardsParticipant& participant) const;
};
/**
 * \ingroup mcptt
 * \brief A class that represents the 'G: Pending Floor Revoke' state.
 */
class McpttOnNetworkFloorArbitratorStateRevoke : public McpttOnNetworkFloorArbitratorState
{
public:
 /**
  * Gets an instance of the McpttOnNetworkFloorArbitratorStateRevoke.
  * \returns An instance.
  */
 static Ptr<McpttOnNetworkFloorArbitratorStateRevoke> GetInstance (void);
 /**
  * \brief Gets the ID of the state.
  * \returns The ID of the state.
  */
 static McpttEntityId GetStateId (void);
 /**
  * \brief Creates an instance of the McpttOnNetworkFloorArbitratorStateRevoke class.
  */
 McpttOnNetworkFloorArbitratorStateRevoke (void);
 /**
  * \brief The destructor of the McpttOnNetworkFloorArbitratorStateRevoke class.
  */
 virtual ~McpttOnNetworkFloorArbitratorStateRevoke (void);
 /**
  * Gets the ID of the machine state.
  * \returns The ID of the state.
  */
 virtual McpttEntityId GetInstanceStateId (void) const;
 /**
  * Indicates if a client is permitted to send media.
  * \returns True, if a client is permited; owtherwise, false.
  */
 virtual bool IsFloorOccupied (const McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Enters the given state.
  * \param machine The FSM.
  */
 virtual void Enter (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Receives a media message.
  * \param machine The FSM.
  * \param msg The received message.
  */
 virtual void ReceiveMedia (McpttOnNetworkFloorArbitrator& machine, const McpttMediaMsg& msg) const;
 /**
  * Receives a floor release message.
  * \param machine The FSM.
  * \param msg The received message.
  */
 virtual void ReceiveFloorRelease (McpttOnNetworkFloorArbitrator& machine, const McpttFloorMsgRelease& msg) const;
 /**
  * Notifies the floor machine state that timer T3 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT3 (McpttOnNetworkFloorArbitrator& machine) const;
 /**
  * Notifies the floor machine state that timer T1 has expired.
  * \param machine The FSM.
  */
 virtual void ExpiryOfT1 (McpttOnNetworkFloorArbitrator& machine) const;
};
/**
 * \ingroup mcptt
 * \brief A class that represents the 'Releasing' state.
 */
class McpttOnNetworkFloorArbitratorStateReleasing : public McpttOnNetworkFloorArbitratorState
{
public:
 /**
  * Gets an instance of the McpttOnNetworkFloorArbitratorStateReleasing.
  * \returns An instance.
  */
 static Ptr<McpttOnNetworkFloorArbitratorStateReleasing> GetInstance (void);
 /**
  * \brief Gets the ID of the state.
  * \returns The ID of the state.
  */
 static McpttEntityId GetStateId (void);
 /**
  * \brief Creates an instance of the McpttOnNetworkFloorArbitratorStateReleasing class.
  */
 McpttOnNetworkFloorArbitratorStateReleasing (void);
 /**
  * \brief The destructor of the McpttOnNetworkFloorArbitratorStateReleasing class.
  */
 virtual ~McpttOnNetworkFloorArbitratorStateReleasing (void);
 /**
  * Gets the ID of the machine state.
  * \returns The ID of the state.
  */
 virtual McpttEntityId GetInstanceStateId (void) const;
 /**
  * Indicates to the floor control server that the call has been released (part II).
  * \param machine The FSM.
  */
 virtual void CallRelease2 (McpttOnNetworkFloorArbitrator& machine) const;
};
/**
 * \ingroup mcptt
 * \brief A class that represents the 'G: Floor Initialising' state.
 */
class McpttOnNetworkFloorArbitratorStateInitialising : public McpttOnNetworkFloorArbitratorState
{
public:
 /**
  * Gets an instance of the McpttOnNetworkFloorArbitratorStateInitialising.
  * \returns An instance.
  */
 static Ptr<McpttOnNetworkFloorArbitratorStateInitialising> GetInstance (void);
 /**
  * \brief Gets the ID of the state.
  * \returns The ID of the state.
  */
 static McpttEntityId GetStateId (void);
 /**
  * \brief Creates an instance of the McpttOnNetworkFloorArbitratorStateInitialising class.
  */
 McpttOnNetworkFloorArbitratorStateInitialising (void);
 /**
  * \brief The destructor of the McpttOnNetworkFloorArbitratorStateInitialising class.
  */
 virtual ~McpttOnNetworkFloorArbitratorStateInitialising (void);
 /**
  * Gets the ID of the machine state.
  * \returns The ID of the state.
  */
 virtual McpttEntityId GetInstanceStateId (void) const;
 /**
  * Enters the given state.
  * \param machine The FSM.
  */
 virtual void Enter (McpttOnNetworkFloorArbitrator& machine) const;
 //TODO:
 // 6.3.4.8.3	Receiving a floor request from a constituent MCPTT group (R: mcptt-floor-request)
 // 6.3.4.8.4	All final SIP responses received (R: final SIP responses)
};

} // namespace ns3

#endif /* MCPTT_ON_NETWORK_FLOOR_ARBITRATOR_STATE_H */
