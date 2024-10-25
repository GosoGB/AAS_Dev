/**
 * @file Status.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크 내에서 상태를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-25
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2023-2024
 */




#pragma once

#include <string>



namespace muffin {
    
    class Status
    {
    public:
        enum class Code : uint32_t
        {
            GOOD                                      = 0x00000000,
            UNCERTAIN                                 = 0x40000000,
            BAD                                       = 0x80000000,
            BAD_UNEXPECTED_ERROR                      = 0x80010000,
            BAD_INTERNAL_ERROR                        = 0x80020000,
            BAD_OUT_OF_MEMORY                         = 0x80030000,
            BAD_RESOURCE_UNAVAILABLE                  = 0x80040000,
            BAD_COMMUNICATION_ERROR                   = 0x80050000,
            BAD_ENCODING_ERROR                        = 0x80060000,
            BAD_DECODING_ERROR                        = 0x80070000,
            BAD_ENCODING_LIMITS_EXCEEDED              = 0x80080000,
            BAD_REQUEST_TOO_LARGE                     = 0x80B80000,
            BAD_RESPONSE_TOO_LARGE                    = 0x80B90000,
            BAD_UNKNOWN_RESPONSE                      = 0x80090000,
            BAD_TIMEOUT                               = 0x800A0000,
            BAD_SERVICE_UNSUPPORTED                   = 0x800B0000,           
            BAD_SHUTDOWN                              = 0x800C0000,
            BAD_SERVER_NOT_CONNECTED                  = 0x800D0000,           
            BAD_SERVER_HALTED                         = 0x800E0000,   
            BAD_NOTHING_TO_DO                         = 0x800F0000,  
            BAD_TOO_MANY_OPERATIONS                   = 0x80100000,            
            BAD_TOO_MANY_MONITORED_ITEMS              = 0x80DB0000,            
            BAD_DATATYPE_ID_UNKNOWN                   = 0x80110000,      
            BAD_CERTIFICATE_INVALID                   = 0x80120000,        
            BAD_SECURITY_CHECKS_FAILED                = 0x80130000,      
            BAD_CERTIFICATE_POLICYCHECK_FAILED        = 0x81140000,             
            BAD_CERTIFICATE_TIME_INVALID              = 0x80140000,
            BAD_CERTIFICATE_ISSUER_TIME_INVALID       = 0x80150000,
            BAD_CERTIFICATE_HOSTNAME_INVALID          = 0x80160000,
            BAD_CERTIFICATE_URI_INVALID               = 0x80170000,
            BAD_CERTIFICATE_USE_NOT_ALLOWED           = 0x80180000,
            BAD_CERTIFICATE_ISSUER_USE_NOT_ALLOWED    = 0x80190000,
            BAD_CERTIFICATE_UNTRUSTED                 = 0x801A0000,
            BAD_CERTIFICATE_REVOCATION_UNKNOWN        = 0x801B0000,
            BAD_CERTIFICATE_ISSUER_REVOCATION_UNKNOWN = 0x801C0000,
            BAD_CERTIFICATE_REVOKED                   = 0x801D0000,
            BAD_CERTIFICATE_ISSUER_REVOKED            = 0x801E0000,
            BAD_CERTIFICATE_CHAIN_INCOMPLETE          = 0x810D0000,
            BAD_USER_ACCESS_DENIED                    = 0x801F0000,
            BAD_IDENTITY_TOKEN_INVALID                = 0x80200000,
            BAD_IDENTITY_TOKEN_REJECTED               = 0x80210000,
            BAD_SECURE_CHANNEL_IDINVALID              = 0x80220000,
            BAD_INVALID_TIMESTAMP                     = 0x80230000,
            BAD_NONCE_INVALID                         = 0x80240000,
            BAD_SESSION_ID_INVALID                    = 0x80250000,
            BAD_SESSION_CLOSED                        = 0x80260000,
            BAD_SESSION_NOT_ACTIVATED                 = 0x80270000,
            BAD_SUBSCRIPTION_IDINVALID                = 0x80280000,
            BAD_REQUEST_HEADER_INVALID                = 0x802A0000,
            BAD_TIMESTAMP_STORE_TURN_INVALID          = 0x802B0000,
            BAD_REQUEST_CANCELLED_BY_CLIENT           = 0x802C0000,
            BAD_TOO_MANY_ARGUMENTS                    = 0x80E50000,
            BAD_LICENSE_EXPIRED                       = 0x810E0000,
            BAD_LICENSE_LIMITS_EXCEEDE                = 0x810F0000,
            BAD_LICENSE_NOT_AVAILABL                  = 0x81100000,
            GOOD_SUBSCRIPTION_TRANSFERRED             = 0x002D0000,
            GOOD_COMPLETES_ASYNCHRONOUSLY             = 0x002E0000,
            GOOD_OVERLOAD                             = 0x002F0000,
            GOOD_CLAMPED                              = 0x00300000,
            BAD_NO_COMMUNICATION                      = 0x80310000,
            BAD_WAITING_FOR_INITIAL_DATA              = 0x80320000,
            BAD_NODE_ID_INVALID                       = 0x80330000,
            BAD_NODE_ID_UNKNOWN                       = 0x80340000,
            BAD_ATTRIBUTE_ID_INVALID                  = 0x80350000,
            BAD_INDEX_RANGE_INVALID                   = 0x80360000,
            BAD_INDEX_RANGE_NODATA                    = 0x80370000,
            BAD_DATA_ENCODING_INVALID                 = 0x80380000,
            BAD_DATA_ENCODING_UNSUPPORTED             = 0x80390000,
            BAD_NOT_READABLE                          = 0x803A0000,
            BAD_NOT_WRITABLE                          = 0x803B0000,
            BAD_OUT_OF_RANGE                          = 0x803C0000,
            BAD_NOT_SUPPORTED                         = 0x803D0000,
            BAD_NOT_FOUND                             = 0x803E0000,
            BAD_OBJECT_DELETED                        = 0x803F0000,
            BAD_NOT_IMPLEMENTED                       = 0x80400000,
            BAD_MONITORING_MODE_INVALID               = 0x80410000,
            BAD_MONITORED_ITEM_ID_INVALID             = 0x80420000,
            BAD_MONITORED_ITEM_FILTER_INVALID         = 0x80430000,
            BAD_MONITORED_ITEM_FILTER_UNSUPPORTED     = 0x80440000,
            BAD_FILTER_NOTALLOWED                     = 0x80450000,
            BAD_STRUCT_UREMISSING                     = 0x80460000,
            BAD_EVENT_FILTER_INVALID                  = 0x80470000,
            BAD_CONTENT_FILTER_INVALID                = 0x80480000,
            BAD_FILTER_OPERATOR_INVALID               = 0x80C10000,
            BAD_FILTER_OPERATOR_UNSUPPORTED           = 0x80C20000,
            BAD_FILTER_OPER_AND_COUNT_MISMATCH        = 0x80C30000,
            BAD_FILTER_OPER_AND_INVALID               = 0x80490000,
            BAD_FILTER_ELEMENT_INVALID                = 0x80C40000,
            BAD_FILTER_LITERAL_INVALID                = 0x80C50000,
            BAD_CONTINUATION_POINT_INVALID            = 0x804A0000,
            BAD_NO_CONTINUATION_POINTS                = 0x804B0000,
            BAD_REFERENCE_TYPE_IDINVALID              = 0x804C0000,
            BAD_BROWSED_IRECTION_INVALID              = 0x804D0000,
            BAD_NODE_NOT_INVIEW                       = 0x804E0000,
            BAD_NUMERIC_OVERFLOW                      = 0x81120000,
            BAD_SERVER_URI_INVALID                    = 0x804F0000,
            BAD_SERVER_NAME_MISSING                   = 0x80500000,
            BAD_DISCOVERY_URL_MISSING                 = 0x80510000,
            BAD_SEMPAHORE_FILE_MISSING                = 0x80520000,
            BAD_REQUEST_TYPE_INVALID                  = 0x80530000,
            BAD_SECURITY_MODE_REJECTED                = 0x80540000,
            BAD_SECURITY_POLICY_REJECTED              = 0x80550000,
            BAD_TOO_MANY_SESSIONS                     = 0x80560000,
            BAD_USER_SIGNATURE_INVALID                = 0x80570000,
            BAD_APPLICATION_SIGNATURE_INVALID         = 0x80580000,
            BAD_NO_VALID_CERTIFICATES                 = 0x80590000,
            BAD_IDENTITY_CHANGE_NOT_SUPPORTED         = 0x80C60000,
            BAD_REQUEST_CANCELLED_BY_REQUEST          = 0x805A0000,
            BAD_PARENT_NODE_ID_INVALID                = 0x805B0000,
            BAD_REFERENCE_NOT_ALLOWED                 = 0x805C0000,
            BAD_NODE_ID_REJECTED                      = 0x805D0000,
            BAD_NODE_ID_EXISTS                        = 0x805E0000,
            BAD_NODE_CLASS_INVALID                    = 0x805F0000,
            BAD_BROWSE_NAME_INVALID                   = 0x80600000,
            BAD_BROWSE_NAME_DUPLICATED                = 0x80610000,
            BAD_NODE_ATTRIBUTES_INVALID               = 0x80620000,
            BAD_TYPEDEFINITION_INVALID                = 0x80630000,
            BAD_SOURCE_NODE_ID_INVALID                = 0x80640000,
            BAD_TARGET_NODE_ID_INVALID                = 0x80650000,
            BAD_DUPLICATE_REFERENCE_NOT_ALLOWED       = 0x80660000,
            BAD_INVALID_SELF_REFERENCE                = 0x80670000,
            BAD_REFERENCE_LOCAL_ONLY                  = 0x80680000,
            BAD_NODE_DELETE_RIGHTS                    = 0x80690000,
            UNCERTAIN_REFERENCE_NOT_DELETED           = 0x40BC0000,
            BAD_SERVER_INDEX_INVALID                  = 0x806A0000,
            BAD_VIEW_ID_UNKNOWN                       = 0x806B0000,
            BAD_VIEW_TIMESTAMP_INVALID                = 0x80C90000,
            BAD_VIEW_PARAMETER_MISMATCH               = 0x80CA0000,
            BAD_VIEW_VERSION_INVALID                  = 0x80CB0000,
            UNCERTAIN_NOT_ALL_NODES_AVAILABLE         = 0x40C00000,
            GOOD_RESULTS_MAYBE_INCOMPLETE             = 0x00BA0000,
            BAD_NOT_TYPEDEFINITION                    = 0x80C80000,
            UNCERTAIN_REFERENCE_OUT_OF_SERVER         = 0x406C0000,
            BAD_TOO_MANY_MATCHES                      = 0x806D0000,
            BAD_QUERY_TOO_COMPLEX                     = 0x806E0000,
            BAD_NO_MATCH                              = 0x806F0000,
            BAD_MAX_AGE_INVALID                       = 0x80700000,
            BAD_SECURITY_MODE_INSUFFICIENT            = 0x80E60000,
            BAD_HISTORY_OPERATION_INVALID             = 0x80710000,
            BAD_HISTORY_OPERATION_UNSUPPORTED         = 0x80720000,
            BAD_INVALID_TIMESTAMP_ARGUMENT            = 0x80BD0000,
            BAD_WRITE_NOT_SUPPORTED                   = 0x80730000,
            BAD_TYPE_MISMATCH                         = 0x80740000,
            BAD_METHOD_INVALID                        = 0x80750000,
            BAD_ARGUMENTS_MISSING                     = 0x80760000,
            BAD_NOT_EXECUTABLE                        = 0x81110000,
            BAD_TOO_MANY_SUBSCRIPTIONS                = 0x80770000,
            BAD_TOO_MANY_PUBLISH_REQUESTS             = 0x80780000,
            BAD_NO_SUBSCRIPTION                       = 0x80790000,
            BAD_SEQUENCE_NUMBER_UNKNOWN               = 0x807A0000,
            GOOD_RETRANSMISSION_QUEUE_NOT_SUPPORTED   = 0x00DF0000,
            BAD_MESSAGE_NOT_AVAILABLE                 = 0x807B0000,
            BAD_INSUFFICIENT_CLIENT_PROFILE           = 0x807C0000,
            BAD_STATE_NOT_ACTIVE                      = 0x80BF0000,
            BAD_ALREADY_EXISTS                        = 0x81150000,
            BAD_TCP_SERVER_TOO_BUSY                     = 0x807D0000,
            BAD_TCP_MESSAGE_TYPE_INVALID                = 0x807E0000,
            BAD_TCP_SECURE_CHANNEL_UNKNOWN              = 0x807F0000,
            BAD_TCP_MESSAGE_TOO_LARGE                   = 0x80800000,
            BAD_TCP_NOT_ENOUGH_RESOURCES                = 0x80810000,
            BAD_TCP_INTERNAL_ERROR                      = 0x80820000,
            BAD_TCP_ENDPOINT_URL_INVALID                = 0x80830000,
            BAD_REQUEST_INTERRUPTED                     = 0x80840000,
            BAD_REQUEST_TIMEOUT                         = 0x80850000,
            BAD_SECURE_CHANNEL_CLOSED                   = 0x80860000,
            BAD_SECURE_CHANNEL_TOKEN_UNKNOWN            = 0x80870000,
            BAD_SEQUENCE_NUMBER_INVALID                 = 0x80880000,
            BAD_PROTOCOL_VERSION_UNSUPPORTED            = 0x80BE0000,
            BAD_CONFIGURATION_ERROR                     = 0x80890000,
            BAD_NOT_CONNECTED                           = 0x808A0000,
            BAD_DEVICE_FAILURE                          = 0x808B0000,
            BAD_SENSOR_FAILURE                          = 0x808C0000,
            BAD_OUT_OF_SERVICE                          = 0x808D0000,
            BAD_DEADBAND_FILTER_INVALID                 = 0x808E0000,
            UNCERTAIN_NO_COMMUNICATION_LAST_USABLE_VALUE= 0x408F0000,
            UNCERTAIN_LAST_USABLE_VALUE                 = 0x40900000,
            UNCERTAIN_SUBSTITUTE_VALUE                  = 0x40910000,
            UNCERTAIN_INITIAL_VALUE                     = 0x40920000,
            UNCERTAIN_SENSOR_NOT_ACCURATE               = 0x40930000,
            UNCERTAIN_ENGINEERING_UNITS_EXCEEDED        = 0x40940000,
            UNCERTAIN_SUBNORMAL                         = 0x40950000,
            GOOD_LOCAL_OVERRIDE                         = 0x00960000,
            BAD_REFRESH_IN_PROGRESS                     = 0x80970000,
            BAD_CONDITION_ALREADY_DISABLED              = 0x80980000,
            BAD_CONDITION_ALREADY_ENABLED               = 0x80CC0000,
            BAD_CONDITION_DISABLED                      = 0x80990000,
            BAD_EVENT_ID_UNKNOWN                        = 0x809A0000,
            BAD_EVENT_NOT_ACKNOWLEDGEABLE               = 0x80BB0000,
            BAD_DIALOG_NOT_ACTIVE                       = 0x80CD0000,
            BAD_DIALOG_RESPONSE_INVALID                 = 0x80CE0000,
            BAD_CONDITION_BRANCH_ALREADY_ACKED          = 0x80CF0000,
            BAD_CONDITION_BRANCH_ALREADY_CONFIRMED      = 0x80D00000,
            BAD_CONDITIONAL_READY_SHELVED               = 0x80D10000,
            BAD_CONDITION_NOT_SHELVED                   = 0x80D20000,
            BAD_SHELVING_TIMEOUT_OUT_OF_RANGE           = 0x80D30000,
            BAD_NO_DATA                                 = 0x809B0000,
            BAD_BOUND_NOT_FOUND                         = 0x80D70000,
            BAD_BOUND_NOT_SUPPORTED                     = 0x80D80000,
            BAD_DATA_LOST                               = 0x809D0000,
            BAD_DATA_UNAVAILABLE                        = 0x809E0000,
            BAD_ENTRY_EXISTS                            = 0x809F0000,
            BAD_NO_ENTRY_EXISTS                         = 0x80A00000,
            BAD_TIMESTAMP_NOT_SUPPORTED                 = 0x80A10000,
            GOOD_ENTRY_INSERTED                         = 0x00A20000,
            GOOD_ENTRY_REPLACED                         = 0x00A30000,
            UNCERTAIN_DATA_SUBNORMAL                    = 0x40A40000,
            GOOD_NO_DATA                                = 0x00A50000,
            GOOD_MORE_DATA                              = 0x00A60000,
            BAD_AGGREGATE_LIST_MISMATCH                 = 0x80D40000,
            BAD_AGGREGATE_NOT_SUPPORTED                 = 0x80D50000,
            BAD_AGGREGATE_INVALID_INPUTS                = 0x80D60000,
            BAD_AGGREGATE_CONFIGURATION_REJECTED        = 0x80DA0000,
            GOOD_DATA_IGNORED                           = 0x00D90000,
            BAD_REQUEST_NOT_ALLOWED                     = 0x80E40000,
            BAD_REQUEST_NOT_COMPLETE                    = 0x81130000,
            BAD_TICKET_REQUIRED                         = 0x811F0000,
            BAD_TICKET_INVALID                          = 0x81200000,
            GOOD_EDITED                                 = 0x00DC0000,
            GOOD_POST_ACTION_FAILED                     = 0x00DD0000,
            UNCERTAIN_DOMINANT_VALUE_CHANGED            = 0x40DE0000,
            GOOD_DEPENDENT_VALUE_CHANGED                = 0x00E00000,
            BAD_DOMINANT_VALUE_CHANGED                  = 0x80E10000,
            UNCERTAIN_DEPENDENT_VALUE_CHANGED           = 0x40E20000,
            BAD_DEPENDENT_VALUE_CHANGED                 = 0x80E30000,
            GOOD_EDITED_DEPENDENT_VALUE_CHANGED         = 0x01160000,
            GOOD_EDITED_DOMINANT_VALUE_CHANGED          = 0x01170000,
            BAD_EDITED_OUT_OF_RANGE                     = 0x81190000,
            BAD_INITIAL_VALUE_OUT_OF_RANGE              = 0x811A0000,
            BAD_OUT_OF_RANGE_DOMINANT_VALUE_CHANGED     = 0x811B0000,
            GOOD_COMMUNICATION_EVENT                    = 0x00A70000,
            GOOD_SHUTDOWN_EVENT                         = 0x00A80000,
            GOOD_CALL_AGAIN                             = 0x00A90000,
            GOOD_NON_CRITICAL_TIMEOUT                   = 0x00AA0000,
            BAD_INVALID_ARGUMENT                        = 0x80AB0000,
            BAD_CONNECTION_REJECTED                     = 0x80AC0000,
            BAD_DISCONNECT                              = 0x80AD0000,
            BAD_CONNECTION_CLOSED                       = 0x80AE0000,
            BAD_INVALID_STATE                           = 0x80AF0000,
            BAD_END_OF_STREAM                           = 0x80B00000,
            BAD_NO_DATA_AVAILABLE                       = 0x80B10000,
            BAD_WAITING_FOR_RESPONSE                    = 0x80B20000,
            BAD_OPERATION_ABANDONED                     = 0x80B30000,
            BAD_EXPECTED_STREAM_TO_BLOCK                = 0x80B40000,
            BAD_WOULD_BLOCK                             = 0x80B50000,
            BAD_SYNTAX_ERROR                            = 0x80B60000,
            BAD_MAX_CONNECTIONS_REACHED                 = 0x80B70000,
            GOOD_EDITED_DOMINANT_VALUE_CHANGED_DEPENDENT_VALUE_CHANGED             = 0x01180000,
            BAD_EDITED_OUT_OF_RANGE_DOMINANT_VALUE_CHANGED                         = 0x811C0000,
            BAD_OUT_OF_RANGE_DOMINANT_VALUE_CHANGED_DEPENDENT_VALUE_CHANGED        = 0x811D0000,
            BAD_EDITED_OUT_OF_RANGE_DOMINANT_VALUE_CHANGED_DEPENDENT_VALUE_CHANGED = 0x811E0000
        };


    public:
        explicit Status(Code code) : mCode(code) {}
        virtual ~Status() {}
        Status& operator=(const Status& obj)
        {
            if (this != &obj)
            {
                mCode = obj.ToCode();
            }

            return *this;
        }
        Status& operator=(const Code& obj)
        {
            mCode = obj;
            return *this;
        }
        bool operator==(const Status& obj) { return mCode == obj.ToCode(); }
        bool operator!=(const Status& obj) { return mCode != obj.ToCode(); }
        bool operator==(const Code& obj)   { return mCode == obj; }
        bool operator!=(const Code& obj)   { return mCode != obj; }
    public:
        const char* c_str() const;
        std::string ToString() const;
        Code ToCode() const;

    private:
        Code mCode;
    };
}