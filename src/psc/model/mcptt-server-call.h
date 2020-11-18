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

#ifndef MCPTT_SERVER_CALL_H
#define MCPTT_SERVER_CALL_H

#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/type-id.h>

#include "mcptt-call-msg.h"
#include "mcptt-channel.h"
#include "mcptt-floor-participant.h"
#include "mcptt-floor-msg.h"
#include "mcptt-media-msg.h"

namespace ns3 {

class McpttServerApp;
class McpttOnNetworkFloorArbitrator;
class McpttServerCallMachine;
class SipHeader;

/**
 * \ingroup mcptt
 *
 * This class used to encapsulate the state to represent an MCPTT call on the
 * MCPTT server.  Associated with a call is a call ID, to identify
 * the particular call, a call machine to perform call control, an arbitrator
 * (floor control server), a floor control message channel, and a media
 * message channel. An MCPTT server call should be subordinate of an MCPTT 
 * server application.
 *
 * The corresponding call encapsulation class on the MCPTT clients is the
 * class ns3::McpttCall.
 */
class McpttServerCall : public Object
{
public:
 /**
  * Gets the type ID of the McpttServerCall class.
  * \returns The type ID.
  */
 static TypeId GetTypeId (void);
 /**
  * Creates an instance of the McpttServerCall class.
  */
  McpttServerCall (void);
 /**
  * The destructor of the McpttServerCall class.
  */
 virtual ~McpttServerCall (void);
 /**
  * Sets the ID of the call.
  * \param callId The call ID.
  */
 void SetCallId (uint16_t callId);
 /**
  * Gets the ID of the call.
  * \returns The call ID.
  */
 uint16_t GetCallId (void) const;
 /**
  * Indicates if the call is configured for ambient listening.
  * \returns True, if the call is configured for ambient listening; otherwise, false.
  */
 virtual bool IsAmbientListening (void) const;
 /**
  * Indicates if the call is configured for a temporary group session.
  * \returns True, if the call is configured for a temporary group session; false, otherwise.
  */
 virtual bool IsTemporaryGroup (void) const;
 /**
  * Indicates if the floor channel is open.
  * \returns True, if the channel is open.
  */
 bool IsFloorChannelOpen (void) const;
 /**
  * Indicates if the media channel is open.
  * \returns True, if the channel is open.
  */
 bool IsMediaChannelOpen (void) const;
 /**
  * Opens the floor channel.
  * \param peerAddr The peer address.
  * \param port The peer port.
  */
 void OpenFloorChannel (const Address& peerAddr, const uint16_t port);
 /**
  * Opens the media channel.
  * \param peerAddr The peer address.
  * \param port The peer port.
  */
 void OpenMediaChannel (const Address& peerAddr, const uint16_t port);
 /**
  * Receives a call control packet.
  * \param pkt The packet (without SIP header) that was received.
  * \param hdr The (deserialized) SIP header.
  */
 virtual void ReceiveCallPacket (Ptr<Packet> pkt, const SipHeader& hdr);
 /**
  * Receives a floor message.
  * \param msg The message that was received.
  */
 virtual void Receive (const McpttFloorMsg& msg);
 /**
  * Receive a media message.
  * \param msg The message that was received.
  */
 virtual void Receive (const McpttMediaMsg& msg);
 /**
  * Sends a floor message.
  * \param msg The message to send.
  */
 void Send (const McpttFloorMsg& msg);
 /**
  * Sends a media message.
  * \param msg The media message.
  */
 void Send (const McpttMediaMsg& msg);
 /**
  * Sends a call control packet.
  * \param pkt The packet (already serialized with SIP header)
  * \param toAddr The address to send to
  * \param hdr A reference to the SIP header that has been serialized
  */
 virtual void SendCallControlPacket (Ptr<Packet> pkt, const Address& toAddr, const SipHeader &hdr);
protected:
 /**
  * Disposes of the McpttServerCall instance.
  */
 void DoDispose (void);
 /**
  * Handles the receieved floor control packet.
  * \param pkt The packet that was received.
  */
 void ReceiveFloorPkt (Ptr<Packet>  pkt);
 /**
  * Handles the received media packet.
  * \param pkt The packet that was received.
  */
 void ReceiveMediaPkt (Ptr<Packet>  pkt);
 private:
 uint16_t m_callId;  //!< Call ID of the call.
 std::vector<uint32_t> m_clientUserIds; //!< Client user IDs
 uint32_t m_originator;  //!< Originating UE user ID
 bool m_ambientListening; //!< The flag that indicates if the call is configured for ambient listening.
 bool m_temporaryGroup; //!< The flag that indicates if the call is configured for a temporary group.
 Ptr<McpttChannel> m_floorChannel; //!< The channel to use for floor control messages.
 Ptr<McpttServerCallMachine> m_callMachine; //!< The call control machine.
 Ptr<McpttOnNetworkFloorArbitrator> m_arbitrator; //!< The floor control machine.
 Ptr<McpttChannel> m_mediaChannel; //!< The channel to use for media messages.
 Ptr<McpttServerApp> m_owner; //!< The owner of this call.
 Callback<void, Ptr<const McpttServerCall>, const Header&> m_rxCb; //!< The received message callback.
 Callback<void, Ptr<const McpttServerCall>, const Header&> m_txCb; //!< The transmitted message callback.
public:
 /**
  * Gets the call control state machine.
  * \returns The call machine.
  */
 Ptr<McpttServerCallMachine> GetCallMachine (void) const;
 /**
  * Gets the channel to use for floor control messages.
  * \returns The channel.
  */
 Ptr<McpttChannel> GetFloorChannel (void) const;
 /**
  * Gets the arbitrator.
  * \returns The arbitrator
  */
 Ptr<McpttOnNetworkFloorArbitrator> GetArbitrator (void) const;
 /**
  * Gets the channel to use for floor control messages.
  * \returns The channel.
  */
 Ptr<McpttChannel> GetMediaChannel (void) const;
 /**
  * Gets the owner of this call.
  * \returns The owner.
  */
 Ptr<McpttServerApp> GetOwner (void) const;
 /**
  * Sets the call control state machine.
  * \param callMachine The call control state machine.
  */
 void SetCallMachine (Ptr<McpttServerCallMachine> callMachine);
 /**
  * Sets the channel to use for floor control messages.
  * \param floorChannel The channel.
  */
 void SetFloorChannel (Ptr<McpttChannel>  floorChannel);
 /**
  * Sets the arbitrator.
  * \param arbitrator The arbitrator
  */
 void SetArbitrator (Ptr<McpttOnNetworkFloorArbitrator>  arbitrator);
 /**
  * Sets the channel to use for media messages.
  * \param mediaChannel The channel.
  */
 void SetMediaChannel (Ptr<McpttChannel>  mediaChannel);
 /**
  * Sets the owner of this call.
  * \param owner The owner.
  */
 void SetOwner (Ptr<McpttServerApp> owner);
 /**
  * Sets the list of client MCPTT user IDs belonging to this call
  * \param clientUserIds the client UserIds
  */
 void SetClientUserIds (std::vector<uint32_t> clientUserIds);
 /**
  * Gets the list of client MCPTT user IDs belonging to this call
  * \return the client UserIds
  */
 std::vector<uint32_t> GetClientUserIds (void) const;
 /**
  * Sets the originating client MCPTT user IDs belonging to this call
  * \param originator the originating client's user ID
  */
 void SetOriginator (uint32_t originator);
 /**
  * Gets the originating client MCPTT user IDs for this call
  * \param originator the originating client's user ID
  */
 uint32_t GetOriginator (void) const;
 /**
  * Sets the received message callback.
  * \param rxCb The callback.
  */
 void SetRxCb (const Callback<void, Ptr<const McpttServerCall>, const Header&>  rxCb);
 /**
  * Sets the transmitted message callback.
  * \param txCb The callback.
  */
 void SetTxCb (const Callback<void, Ptr<const McpttServerCall>, const Header&>  txCb);
};

} // namespace ns3

#endif /* MCPTT_SERVER_CALL_H */

