// /**
//  * @file Message.cpp
//  * @author Lee, Sang-jin (lsj31@edgecross.ai)
//  * 
//  * @brief MQTT 프로토콜의 메시지 클래스를 정의합니다.
//  * 
//  * @date 2024-09-12
//  * @version 0.0.1
//  * 
//  * @copyright Copyright Edgecross Inc. (c) 2024
//  */




// #include <string.h>

// #include "Common/Assert.h"
// #include "Common/Logger/Logger.h"
// #include "Message.h"



// namespace muffin { namespace mqtt {

//     Message::Message(const std::string& mac, topic_e topic, const std::string& payload, const socket_e socketID, const uint16_t messageID, const qos_e qos, const bool isRetain)
//         : mMacAddress(mac)
//         , mSocketID(socketID)
//         , mMessageID(messageID)
//         , mQoS(qos)
//         , mRetainFlag(isRetain)
//         , mTopic(topic)
//         , mPayload(payload)
//     {
//     #if defined(DEBUG)
//         LOG_DEBUG(logger, "--------------------------------------------------");
//         LOG_DEBUG(logger, "Constructed at address: %p", this);
//         LOG_DEBUG(logger, "--------------------------------------------------");
//         LOG_DEBUG(logger, "SocketID: %u", GetSocketID());
//         LOG_DEBUG(logger, "MessageID: %u", GetMessageID());
//         LOG_DEBUG(logger, "QoS Level: %u", GetQoS());
//         LOG_DEBUG(logger, "Retain: %s", IsRetain() ? "true" : "false");
//         LOG_DEBUG(logger, "Topic: %s", GetTopicString());
//         LOG_DEBUG(logger, "Payload: %s", GetPayload());
//         LOG_DEBUG(logger, "--------------------------------------------------\n\n");
//     #endif
//     }

//     Message::Message(const Message& obj)
//         : mMacAddress(obj.mMacAddress)
//         , mSocketID(obj.mSocketID)
//         , mMessageID(obj.mMessageID)
//         , mQoS(obj.mQoS)
//         , mRetainFlag(obj.mRetainFlag)
//         , mTopic(obj.mTopic)
//         , mPayload(obj.mPayload)
//     {
//     #if defined(DEBUG)
//         LOG_DEBUG(logger, "--------------------------------------------------");
//         LOG_DEBUG(logger, "Constructed by Copy from %p to %p", &obj, this);
//         LOG_DEBUG(logger, "--------------------------------------------------");
//         LOG_DEBUG(logger, "SocketID: %u", GetSocketID());
//         LOG_DEBUG(logger, "MessageID: %u", GetMessageID());
//         LOG_DEBUG(logger, "QoS Level: %u", GetQoS());
//         LOG_DEBUG(logger, "Retain: %s", IsRetain() ? "true" : "false");
//         LOG_DEBUG(logger, "Topic: %s", GetTopicString());
//         LOG_DEBUG(logger, "Payload: %s", GetPayload());
//         LOG_DEBUG(logger, "--------------------------------------------------\n\n");
//     #endif
//     }

//     Message::Message(Message&& obj) noexcept
//         : mMacAddress(std::move(obj.mMacAddress))
//         , mSocketID(std::move(obj.mSocketID))
//         , mMessageID(std::move(obj.mMessageID))
//         , mQoS(std::move(obj.mQoS))
//         , mRetainFlag(std::move(obj.mRetainFlag))
//         , mTopic(std::move(obj.mTopic))
//         , mPayload(std::move(obj.mPayload))
//     {
//     #if defined(DEBUG)
//         LOG_DEBUG(logger, "--------------------------------------------------");
//         LOG_DEBUG(logger, "Constructed by Move from %p to %p", &obj, this);
//         LOG_DEBUG(logger, "--------------------------------------------------");
//         LOG_DEBUG(logger, "SocketID: %u", GetSocketID());
//         LOG_DEBUG(logger, "MessageID: %u", GetMessageID());
//         LOG_DEBUG(logger, "QoS Level: %u", GetQoS());
//         LOG_DEBUG(logger, "Retain: %s", IsRetain() ? "true" : "false");
//         LOG_DEBUG(logger, "Topic: %s", GetTopicString());
//         LOG_DEBUG(logger, "Payload: %s", GetPayload());
//         LOG_DEBUG(logger, "--------------------------------------------------\n\n");
//     #endif
//     }

//     Message::~Message()
//     {
//     #if defined(DEBUG)
//         LOG_DEBUG(logger, "Destroyed at address: %p", this);
//     #endif
//     }

//     socket_e Message::GetSocketID() const
//     {
//         return mSocketID;
//     }

//     uint16_t Message::GetMessageID() const
//     {
//         return mMessageID;
//     }

//     qos_e Message::GetQoS() const
//     {
//         return mQoS;
//     }

//     bool Message::IsRetain() const
//     {
//         return mRetainFlag;
//     }

//     topic_e Message::GetTopic() const
//     {
//         return mTopic;
//     }

//     const char* Message::GetTopicString() const
//     {
//         switch (mTopic)
//         {
//         case topic_e::LAST_WILL:
//             return "scautr/modlink/status/network/will";
//         case topic_e::JARVIS_REQUEST:
//             return static_cast<const char*>(strcat("mfm/", mMacAddress.c_str()));
//         case topic_e::JARVIS_RESPONSE:
//             return static_cast<const char*>(strcat("mfm/resp/", mMacAddress.c_str()));
//         default:
//             ASSERT(false, "UNDEFINED MQTT TOPIC");
//             return nullptr;
//         }
//     }

//     const char* Message::GetPayload() const
//     {
//         return mPayload.c_str();
//     }

//     topic_e Message::Convert2Topic(const std::string& topic)
//     {
//         return Convert2Topic(topic.c_str());
//     }

//     topic_e Message::Convert2Topic(const char* topic)
//     {
//         ASSERT(
//             (
//                 strcmp(topic, "scautr/modlink/status/network/will") == 0 || 
//                 strcmp(topic, strcat("mfm/", mMacAddress.c_str())) == 0 || 
//                 strcmp(topic, strcat("mfm/resp/", mMacAddress.c_str())) == 0
//             ),
//             "INVALID TOPIC: %s", topic
//         );

//         if (strcmp(topic, "scautr/modlink/status/network/will") == 0)
//         {
//             return topic_e::LAST_WILL;
//         }
//         else if (strcmp(topic, strcat("mfm/", mMacAddress.c_str())) == 0)
//         {
//             return topic_e::JARVIS_REQUEST;
//         }
//         else
//         {
//             return topic_e::JARVIS_RESPONSE;
//         }
//     }
// }}