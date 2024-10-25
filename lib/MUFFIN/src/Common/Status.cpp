/**
 * @file Status.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크 내에서 상태를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-25
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2023-2024
 */




#include "Status.h"



namespace muffin {


    const char* Status::c_str() const
    {
        switch (mCode)
        {
        case Code::GOOD:
            return "GOOD";
        case Code::UNCERTAIN:
            return "UNCERTAIN";
        case Code::BAD:
            return "BAD";
        case Code::BAD_UNEXPECTED_ERROR:
            return "BAD UNEXPECTED ERROR";
        case Code::BAD_INTERNAL_ERROR:
            return "BAD INTERNAL ERROR";
        case Code::BAD_OUT_OF_MEMORY:
            return "BAD OUT OF MEMORY";
        case Code::BAD_RESOURCE_UNAVAILABLE:
            return "BAD RESOURCE UNAVAILABLE";
        case Code::BAD_COMMUNICATION_ERROR:
            return "BAD COMMUNICATION ERROR";
        case Code::BAD_ENCODING_ERROR:
            return "UFFIN STATUS BAD ENCODING ERROR";
        case Code::BAD_DECODING_ERROR:
            return "BAD DECODING ERROR";
        case Code::BAD_ENCODING_LIMITS_EXCEEDED:
            return "BAD ENCODING LIMITS EXCEEDED";
        case Code::BAD_REQUEST_TOO_LARGE:
            return "BAD REQUEST TOO LARGE";
        case Code::BAD_RESPONSE_TOO_LARGE:
            return "BAD RESPONSE TOO LARGE";
        case Code::BAD_UNKNOWN_RESPONSE:
            return "BAD UNKNOWN RESPONSE";
        case Code::BAD_TIMEOUT:
            return "BAD TIMEOUT";
        case Code::BAD_SERVICE_UNSUPPORTED:
            return "BAD SERVICE UNSUPPORTED";
        case Code::BAD_SHUTDOWN:
            return "BAD SHUTDOWN";
        case Code::BAD_SERVER_NOT_CONNECTED:
            return "BAD SERVER NOT CONNECTED";
        case Code::BAD_SERVER_HALTED:
            return "BAD SERVER HALTED";
        case Code::BAD_NOTHING_TO_DO:
            return "BAD NOTHING TO DO";
        case Code::BAD_TOO_MANY_OPERATIONS:
            return "BAD TOO MANY OPERATIONS";
        case Code::BAD_TOO_MANY_MONITORED_ITEMS:
            return "BAD TOO MANY MONITORED ITEMS";
        case Code::BAD_DATATYPE_ID_UNKNOWN:
            return "BAD DATATYPE ID UNKNOWN";
        case Code::BAD_CERTIFICATE_INVALID:
            return "BAD CERTIFICATE INVALID";
        case Code::BAD_SECURITY_CHECKS_FAILED:
            return "BAD SECURITY CHECKS FAILED";
        case Code::BAD_CERTIFICATE_POLICYCHECK_FAILED:
            return "BAD CERTIFICATE POLICYCHECK FAILED";
        case Code::BAD_CERTIFICATE_TIME_INVALID:
            return "BAD CERTIFICATE TIME INVALID";
        case Code::BAD_CERTIFICATE_ISSUER_TIME_INVALID:
            return "BAD CERTIFICATE ISSUER TIME INVALID";
        case Code::BAD_CERTIFICATE_HOSTNAME_INVALID:
            return "BAD CERTIFICATE HOSTNAME INVALID";
        case Code::BAD_CERTIFICATE_URI_INVALID:
            return "BAD CERTIFICATE URI INVALID";
        case Code::BAD_CERTIFICATE_USE_NOT_ALLOWED:
            return "BAD CERTIFICATE USE NOT ALLOWED";
        case Code::BAD_CERTIFICATE_ISSUER_USE_NOT_ALLOWED:
            return "BAD CERTIFICATE ISSUER USE NOT ALLOWE";
        case Code::BAD_CERTIFICATE_UNTRUSTED:
            return "BAD CERTIFICATE UNTRUSTED";
        case Code::BAD_CERTIFICATE_REVOCATION_UNKNOWN:
            return "BAD CERTIFICATE REVOCATION UNKNOWN";
        case Code::BAD_CERTIFICATE_ISSUER_REVOCATION_UNKNOWN:
            return "BAD CERTIFICATE ISSUER REVOCATION UNKNOWN";
        case Code::BAD_CERTIFICATE_REVOKED:
            return "BAD CERTIFICATE REVOKED";
        case Code::BAD_CERTIFICATE_ISSUER_REVOKED:
            return "BAD CERTIFICATE ISSUER REVOKED";
        case Code::BAD_CERTIFICATE_CHAIN_INCOMPLETE:
            return "BAD CERTIFICATE CHAIN INCOMPLETE";
        case Code::BAD_USER_ACCESS_DENIED:
            return "BAD USER ACCESS DENIED";
        case Code::BAD_IDENTITY_TOKEN_INVALID:
            return "BAD IDENTITY TOKEN INVALID";
        case Code::BAD_IDENTITY_TOKEN_REJECTED:
            return "BAD IDENTITY TOKEN REJECTED";
        case Code::BAD_SECURE_CHANNEL_IDINVALID:
            return "BAD SECURE CHANNEL IDINVALID";
        case Code::BAD_INVALID_TIMESTAMP:
            return "BAD INVALID TIMESTAMP";
        case Code::BAD_NONCE_INVALID:
            return "BAD NONCE INVALID";
        case Code::BAD_SESSION_ID_INVALID:
            return "BAD SESSION ID INVALID";
        case Code::BAD_SESSION_CLOSED:
            return "BAD SESSION CLOSED";
        case Code::BAD_SESSION_NOT_ACTIVATED:
            return "BAD SESSION NOT ACTIVATED";
        case Code::BAD_SUBSCRIPTION_IDINVALID:
            return "BAD SUBSCRIPTION IDINVALID";
        case Code::BAD_REQUEST_HEADER_INVALID:
            return "BAD REQUEST HEADER INVALID";
        case Code::BAD_TIMESTAMP_STORE_TURN_INVALID:
            return "BAD TIMESTAMP STORE TURN INVALID";
        case Code::BAD_REQUEST_CANCELLED_BY_CLIENT:
            return "BAD REQUEST CANCELLED BY CLIENT";
        case Code::BAD_TOO_MANY_ARGUMENTS:
            return "BAD TOO MANY ARGUMENTS";
        case Code::BAD_LICENSE_EXPIRED:
            return "BAD LICENSE EXPIRED";
        case Code::BAD_LICENSE_LIMITS_EXCEEDE:
            return "BAD LICENSE LIMITS EXCEEDE";
        case Code::BAD_LICENSE_NOT_AVAILABL:
            return "BAD LICENSE NOT AVAILABL";
        case Code::GOOD_SUBSCRIPTION_TRANSFERRED:
            return "GOOD SUBSCRIPTION TRANSFERRED";
        case Code::GOOD_COMPLETES_ASYNCHRONOUSLY:
            return "GOOD COMPLETES ASYNCHRONOUSLY";
        case Code::GOOD_OVERLOAD:
            return "GOOD OVERLOAD";
        case Code::GOOD_CLAMPED:
            return "GOOD CLAMPED";
        case Code::BAD_NO_COMMUNICATION:
            return "BAD NO COMMUNICATION";
        case Code::BAD_WAITING_FOR_INITIAL_DATA:
            return "BAD WAITING FOR INITIAL DATA";
        case Code::BAD_NODE_ID_INVALID:
            return "BAD NODE ID INVALID";
        case Code::BAD_NODE_ID_UNKNOWN:
            return "BAD NODE ID UNKNOWN";
        case Code::BAD_ATTRIBUTE_ID_INVALID:
            return "BAD ATTRIBUTE ID INVALID";
        case Code::BAD_INDEX_RANGE_INVALID:
            return "BAD INDEX RANGE INVALID";
        case Code::BAD_INDEX_RANGE_NODATA:
            return "BAD INDEX RANGE NODATA";
        case Code::BAD_DATA_ENCODING_INVALID:
            return "BAD DATA ENCODING INVALID";
        case Code::BAD_DATA_ENCODING_UNSUPPORTED:
            return "BAD DATA ENCODING UNSUPPORTED";
        case Code::BAD_NOT_READABLE:
            return "BAD NOT READABLE";
        case Code::BAD_NOT_WRITABLE:
            return "BAD NOT WRITABLE";
        case Code::BAD_OUT_OF_RANGE:
            return "BAD OUT OF RANGE";
        case Code::BAD_NOT_SUPPORTED:
            return "BAD NOT SUPPORTED";
        case Code::BAD_NOT_FOUND:
            return "BAD NOT FOUND";
        case Code::BAD_OBJECT_DELETED:
            return "BAD OBJECT DELETED";
        case Code::BAD_NOT_IMPLEMENTED:
            return "BAD NOT IMPLEMENTED";
        case Code::BAD_MONITORING_MODE_INVALID:
            return "BAD MONITORING MODE INVALID";
        case Code::BAD_MONITORED_ITEM_ID_INVALID:
            return "BAD MONITORED ITEM ID INVALID";
        case Code::BAD_MONITORED_ITEM_FILTER_INVALID:
            return "BAD MONITORED ITEM FILTER INVALID";
        case Code::BAD_MONITORED_ITEM_FILTER_UNSUPPORTED:
            return "BAD MONITORED ITEM FILTER UNSUPPORTED";
        case Code::BAD_FILTER_NOTALLOWED:
            return "BAD FILTER NOTALLOWED";
        case Code::BAD_STRUCT_UREMISSING:
            return "BAD STRUCT UREMISSING";
        case Code::BAD_EVENT_FILTER_INVALID:
            return "BAD EVENT FILTER INVALID";
        case Code::BAD_CONTENT_FILTER_INVALID:
            return "BAD CONTENT FILTER INVALID";
        case Code::BAD_FILTER_OPERATOR_INVALID:
            return "BAD FILTER OPERATOR INVALID";
        case Code::BAD_FILTER_OPERATOR_UNSUPPORTED:
            return "BAD FILTER OPERATOR UNSUPPORTED";
        case Code::BAD_FILTER_OPER_AND_COUNT_MISMATCH:
            return "BAD FILTER OPER AND COUNT MISMATCH";
        case Code::BAD_FILTER_OPER_AND_INVALID:
            return "BAD FILTER OPER AND INVALID";
        case Code::BAD_FILTER_ELEMENT_INVALID:
            return "BAD FILTER ELEMENT INVALID";
        case Code::BAD_FILTER_LITERAL_INVALID:
            return "BAD FILTER LITERAL INVALID";
        case Code::BAD_CONTINUATION_POINT_INVALID:
            return "BAD CONTINUATION POINT INVALID";
        case Code::BAD_NO_CONTINUATION_POINTS:
            return "BAD NO CONTINUATION POINTS";
        case Code::BAD_REFERENCE_TYPE_IDINVALID:
            return "BAD REFERENCE TYPE IDINVALID";
        case Code::BAD_BROWSED_IRECTION_INVALID:
            return "BAD BROWSED IRECTION INVALID";
        case Code::BAD_NODE_NOT_INVIEW:
            return "BAD NODE NOT INVIEW";
        case Code::BAD_NUMERIC_OVERFLOW:
            return "BAD NUMERIC OVERFLOW";
        case Code::BAD_SERVER_URI_INVALID:
            return "BAD SERVER URI INVALID";
        case Code::BAD_SERVER_NAME_MISSING:
            return "BAD SERVER NAME MISSING";
        case Code::BAD_DISCOVERY_URL_MISSING:
            return "BAD DISCOVERY URL MISSING";
        case Code::BAD_SEMPAHORE_FILE_MISSING:
            return "BAD SEMPAHORE FILE MISSING";
        case Code::BAD_REQUEST_TYPE_INVALID:
            return "BAD REQUEST TYPE INVALID";
        case Code::BAD_SECURITY_MODE_REJECTED:
            return "BAD SECURITY MODE REJECTED";
        case Code::BAD_SECURITY_POLICY_REJECTED:
            return "BAD SECURITY POLICY REJECTED";
        case Code::BAD_TOO_MANY_SESSIONS:
            return "BAD TOO MANY SESSIONS";
        case Code::BAD_USER_SIGNATURE_INVALID:
            return "BAD USER SIGNATURE INVALID";
        case Code::BAD_APPLICATION_SIGNATURE_INVALID:
            return "BAD APPLICATION SIGNATURE INVALID";
        case Code::BAD_NO_VALID_CERTIFICATES:
            return "BAD NO VALID CERTIFICATES";
        case Code::BAD_IDENTITY_CHANGE_NOT_SUPPORTED:
            return "BAD IDENTITY CHANGE NOT SUPPORTED";
        case Code::BAD_REQUEST_CANCELLED_BY_REQUEST:
            return "BAD REQUEST CANCELLED BY REQUEST";
        case Code::BAD_PARENT_NODE_ID_INVALID:
            return "BAD PARENT NODE ID INVALID";
        case Code::BAD_REFERENCE_NOT_ALLOWED:
            return "BAD REFERENCE NOT ALLOWED";
        case Code::BAD_NODE_ID_REJECTED:
            return "BAD NODE ID REJECTED";
        case Code::BAD_NODE_ID_EXISTS:
            return "BAD NODE ID EXISTS";
        case Code::BAD_NODE_CLASS_INVALID:
            return "BAD NODE CLASS INVALID";
        case Code::BAD_BROWSE_NAME_INVALID:
            return "BAD BROWSE NAME INVALID";
        case Code::BAD_BROWSE_NAME_DUPLICATED:
            return "BAD BROWSE NAME DUPLICATED";
        case Code::BAD_NODE_ATTRIBUTES_INVALID:
            return "BAD NODE ATTRIBUTES INVALID";
        case Code::BAD_TYPEDEFINITION_INVALID:
            return "BAD TYPEDEFINITION INVALID";
        case Code::BAD_SOURCE_NODE_ID_INVALID:
            return "BAD SOURCE NODE ID INVALID";
        case Code::BAD_TARGET_NODE_ID_INVALID:
            return "BAD TARGET NODE ID INVALID";
        case Code::BAD_DUPLICATE_REFERENCE_NOT_ALLOWED:
            return "BAD DUPLICATE REFERENCE NOT ALLOWED";
        case Code::BAD_INVALID_SELF_REFERENCE:
            return "BAD INVALID SELF REFERENCE";
        case Code::BAD_REFERENCE_LOCAL_ONLY:
            return "BAD REFERENCE LOCAL ONLY";
        case Code::BAD_NODE_DELETE_RIGHTS:
            return "BAD NODE DELETE RIGHTS";
        case Code::UNCERTAIN_REFERENCE_NOT_DELETED:
            return "UNCERTAIN REFERENCE NOT DELETED";
        case Code::BAD_SERVER_INDEX_INVALID:
            return "BAD SERVER INDEX INVALID";
        case Code::BAD_VIEW_ID_UNKNOWN:
            return "BAD VIEW ID UNKNOWN";
        case Code::BAD_VIEW_TIMESTAMP_INVALID:
            return "BAD VIEW TIMESTAMP INVALID";
        case Code::BAD_VIEW_PARAMETER_MISMATCH:
            return "BAD VIEW PARAMETER MISMATCH";
        case Code::BAD_VIEW_VERSION_INVALID:
            return "BAD VIEW VERSION INVALID";
        case Code::UNCERTAIN_NOT_ALL_NODES_AVAILABLE:
            return "UNCERTAIN NOT ALL NODES AVAILABLE";
        case Code::GOOD_RESULTS_MAYBE_INCOMPLETE:
            return "GOOD RESULTS MAYBE INCOMPLETE";
        case Code::BAD_NOT_TYPEDEFINITION:
            return "BAD NOT TYPEDEFINITION";
        case Code::UNCERTAIN_REFERENCE_OUT_OF_SERVER:
            return "UNCERTAIN REFERENCE OUT OF SERVER";
        case Code::BAD_TOO_MANY_MATCHES:
            return "BAD TOO MANY MATCHES";
        case Code::BAD_QUERY_TOO_COMPLEX:
            return "BAD QUERY TOO COMPLEX";
        case Code::BAD_NO_MATCH:
            return "BAD NO MATCH";
        case Code::BAD_MAX_AGE_INVALID:
            return "BAD MAX AGE INVALID";
        case Code::BAD_SECURITY_MODE_INSUFFICIENT:
            return "BAD SECURITY MODE INSUFFICIENT";
        case Code::BAD_HISTORY_OPERATION_INVALID:
            return "BAD HISTORY OPERATION INVALID";
        case Code::BAD_HISTORY_OPERATION_UNSUPPORTED:
            return "BAD HISTORY OPERATION UNSUPPORTED";
        case Code::BAD_INVALID_TIMESTAMP_ARGUMENT:
            return "BAD INVALID TIMESTAMP ARGUMENT";
        case Code::BAD_WRITE_NOT_SUPPORTED:
            return "BAD WRITE NOT SUPPORTED";
        case Code::BAD_TYPE_MISMATCH:
            return "BAD TYPE MISMATCH";
        case Code::BAD_METHOD_INVALID:
            return "BAD METHOD INVALID";
        case Code::BAD_ARGUMENTS_MISSING:
            return "BAD ARGUMENTS MISSING";
        case Code::BAD_NOT_EXECUTABLE:
            return "BAD NOT EXECUTABLE";
        case Code::BAD_TOO_MANY_SUBSCRIPTIONS:
            return "BAD TOO MANY SUBSCRIPTIONS";
        case Code::BAD_TOO_MANY_PUBLISH_REQUESTS:
            return "BAD TOO MANY PUBLISH REQUESTS";
        case Code::BAD_NO_SUBSCRIPTION:
            return "BAD NO SUBSCRIPTION";
        case Code::BAD_SEQUENCE_NUMBER_UNKNOWN:
            return "BAD SEQUENCE NUMBER UNKNOWN";
        case Code::GOOD_RETRANSMISSION_QUEUE_NOT_SUPPORTED:
            return "GOOD RETRANSMISSION QUEUE NOT SUPPORTED";
        case Code::BAD_MESSAGE_NOT_AVAILABLE:
            return "BAD MESSAGE NOT AVAILABLE";
        case Code::BAD_INSUFFICIENT_CLIENT_PROFILE:
            return "BAD INSUFFICIENT CLIENT PROFILE";
        case Code::BAD_STATE_NOT_ACTIVE:
            return "BAD STATE NOT ACTIVE";
        case Code::BAD_ALREADY_EXISTS:
            return "BAD ALREADY EXISTS";
        case Code::BAD_TCP_SERVER_TOO_BUSY:
            return "BAD TCP SERVER TOO BUSY";
        case Code::BAD_TCP_MESSAGE_TYPE_INVALID:
            return "BAD TCP MESSAGE TYPE INVALID";
        case Code::BAD_TCP_SECURE_CHANNEL_UNKNOWN:
            return "BAD TCP SECURE CHANNEL UNKNOWN";
        case Code::BAD_TCP_MESSAGE_TOO_LARGE:
            return "BAD TCP MESSAGE TOO LARGE";
        case Code::BAD_TCP_NOT_ENOUGH_RESOURCES:
            return "BAD TCP NOT ENOUGH RESOURCES";
        case Code::BAD_TCP_INTERNAL_ERROR:
            return "BAD TCP INTERNAL ERROR";
        case Code::BAD_TCP_ENDPOINT_URL_INVALID:
            return "BAD TCP ENDPOINT URL INVALID";
        case Code::BAD_REQUEST_INTERRUPTED:
            return "BAD REQUEST INTERRUPTED";
        case Code::BAD_REQUEST_TIMEOUT:
            return "BAD REQUEST TIMEOUT";
        case Code::BAD_SECURE_CHANNEL_CLOSED:
            return "BAD SECURE CHANNEL CLOSED";
        case Code::BAD_SECURE_CHANNEL_TOKEN_UNKNOWN:
            return "BAD SECURE CHANNEL TOKEN UNKNOWN";
        case Code::BAD_SEQUENCE_NUMBER_INVALID:
            return "BAD SEQUENCE NUMBER INVALID";
        case Code::BAD_PROTOCOL_VERSION_UNSUPPORTED:
            return "BAD PROTOCOL VERSION UNSUPPORTED";
        case Code::BAD_CONFIGURATION_ERROR:
            return "BAD CONFIGURATION ERROR";
        case Code::BAD_NOT_CONNECTED:
            return "BAD NOT CONNECTED";
        case Code::BAD_DEVICE_FAILURE:
            return "BAD DEVICE FAILURE";
        case Code::BAD_SENSOR_FAILURE:
            return "BAD SENSOR FAILURE";
        case Code::BAD_OUT_OF_SERVICE:
            return "BAD OUT OF SERVICE";
        case Code::BAD_DEADBAND_FILTER_INVALID:
            return "BAD DEADBAND FILTER INVALID";
        case Code::UNCERTAIN_NO_COMMUNICATION_LAST_USABLE_VALUE:
            return "UNCERTAIN NO COMMUNICATION LAST USABLE VALUE";
        case Code::UNCERTAIN_LAST_USABLE_VALUE:
            return "UNCERTAIN LAST USABLE VALUE";
        case Code::UNCERTAIN_SUBSTITUTE_VALUE:
            return "UNCERTAIN SUBSTITUTE VALUE";
        case Code::UNCERTAIN_INITIAL_VALUE:
            return "UNCERTAIN INITIAL VALUE";
        case Code::UNCERTAIN_SENSOR_NOT_ACCURATE:
            return "UNCERTAIN SENSOR NOT ACCURATE";
        case Code::UNCERTAIN_ENGINEERING_UNITS_EXCEEDED:
            return "UNCERTAIN ENGINEERING UNITS EXCEEDED";
        case Code::UNCERTAIN_SUBNORMAL:
            return "UNCERTAIN SUBNORMAL";
        case Code::GOOD_LOCAL_OVERRIDE:
            return "GOOD LOCAL OVERRIDE";
        case Code::BAD_REFRESH_IN_PROGRESS:
            return "BAD REFRESH IN PROGRESS";
        case Code::BAD_CONDITION_ALREADY_DISABLED:
            return "BAD CONDITION ALREADY DISABLED";
        case Code::BAD_CONDITION_ALREADY_ENABLED:
            return "BAD CONDITION ALREADY ENABLED";
        case Code::BAD_CONDITION_DISABLED:
            return "BAD CONDITION DISABLED";
        case Code::BAD_EVENT_ID_UNKNOWN:
            return "BAD EVENT ID UNKNOWN";
        case Code::BAD_EVENT_NOT_ACKNOWLEDGEABLE:
            return "BAD EVENT NOT ACKNOWLEDGEABLE";
        case Code::BAD_DIALOG_NOT_ACTIVE:
            return "BAD DIALOG NOT ACTIVE";
        case Code::BAD_DIALOG_RESPONSE_INVALID:
            return "BAD DIALOG RESPONSE INVALID";
        case Code::BAD_CONDITION_BRANCH_ALREADY_ACKED:
            return "BAD CONDITION BRANCH ALREADY ACKED";
        case Code::BAD_CONDITION_BRANCH_ALREADY_CONFIRMED:
            return "BAD CONDITION BRANCH ALREADY CONFIRMED";
        case Code::BAD_CONDITIONAL_READY_SHELVED:
            return "BAD CONDITIONAL READY SHELVED";
        case Code::BAD_CONDITION_NOT_SHELVED:
            return "BAD CONDITION NOT SHELVED";
        case Code::BAD_SHELVING_TIMEOUT_OUT_OF_RANGE:
            return "BAD SHELVING TIMEOUT OUT OF RANGE";
        case Code::BAD_NO_DATA:
            return "BAD NO DATA";
        case Code::BAD_BOUND_NOT_FOUND:
            return "BAD BOUND NOT FOUND";
        case Code::BAD_BOUND_NOT_SUPPORTED:
            return "BAD BOUND NOT SUPPORTED";
        case Code::BAD_DATA_LOST:
            return "BAD DATA LOST";
        case Code::BAD_DATA_UNAVAILABLE:
            return "BAD DATA UNAVAILABLE";
        case Code::BAD_ENTRY_EXISTS:
            return "BAD ENTRY EXISTS";
        case Code::BAD_NO_ENTRY_EXISTS:
            return "BAD NO ENTRY EXISTS";
        case Code::BAD_TIMESTAMP_NOT_SUPPORTED:
            return "BAD TIMESTAMP NOT SUPPORTED";
        case Code::GOOD_ENTRY_INSERTED:
            return "GOOD ENTRY INSERTED";
        case Code::GOOD_ENTRY_REPLACED:
            return "GOOD ENTRY REPLACED";
        case Code::UNCERTAIN_DATA_SUBNORMAL:
            return "UNCERTAIN DATA SUBNORMAL";
        case Code::GOOD_NO_DATA:
            return "GOOD NO DATA";
        case Code::GOOD_MORE_DATA:
            return "GOOD MORE DATA";
        case Code::BAD_AGGREGATE_LIST_MISMATCH:
            return "BAD AGGREGATE LIST MISMATCH";
        case Code::BAD_AGGREGATE_NOT_SUPPORTED:
            return "BAD AGGREGATE NOT SUPPORTED";
        case Code::BAD_AGGREGATE_INVALID_INPUTS:
            return "BAD AGGREGATE INVALID INPUTS";
        case Code::BAD_AGGREGATE_CONFIGURATION_REJECTED:
            return "BAD AGGREGATE CONFIGURATION REJECTED";
        case Code::GOOD_DATA_IGNORED:
            return "GOOD DATA IGNORED";
        case Code::BAD_REQUEST_NOT_ALLOWED:
            return "BAD REQUEST NOT ALLOWED";
        case Code::BAD_REQUEST_NOT_COMPLETE:
            return "BAD REQUEST NOT COMPLETE";
        case Code::BAD_TICKET_REQUIRED:
            return "BAD TICKET REQUIRED";
        case Code::BAD_TICKET_INVALID:
            return "BAD TICKET INVALID";
        case Code::GOOD_EDITED:
            return "GOOD EDITED";
        case Code::GOOD_POST_ACTION_FAILED:
            return "GOOD POST ACTION FAILED";
        case Code::UNCERTAIN_DOMINANT_VALUE_CHANGED:
            return "UNCERTAIN DOMINANT VALUE CHANGED";
        case Code::GOOD_DEPENDENT_VALUE_CHANGED:
            return "GOOD DEPENDENT VALUE CHANGED";
        case Code::BAD_DOMINANT_VALUE_CHANGED:
            return "BAD DOMINANT VALUE CHANGED";
        case Code::UNCERTAIN_DEPENDENT_VALUE_CHANGED:
            return "UNCERTAIN DEPENDENT VALUE CHANGED";
        case Code::BAD_DEPENDENT_VALUE_CHANGED:
            return "BAD DEPENDENT VALUE CHANGED";
        case Code::GOOD_EDITED_DEPENDENT_VALUE_CHANGED:
            return "GOOD EDITED DEPENDENT VALUE CHANGED";
        case Code::GOOD_EDITED_DOMINANT_VALUE_CHANGED:
            return "GOOD EDITED DOMINANT VALUE CHANGED";
        case Code::GOOD_EDITED_DOMINANT_VALUE_CHANGED_DEPENDENT_VALUE_CHANGED:
            return "GOOD EDITED DOMINANT VALUE CHANGED DEPENDENT VALUE CHANGED";
        case Code::BAD_EDITED_OUT_OF_RANGE:
            return "BAD EDITED OUT OF RANGE";
        case Code::BAD_INITIAL_VALUE_OUT_OF_RANGE:
            return "BAD INITIAL VALUE OUT OF RANGE";
        case Code::BAD_OUT_OF_RANGE_DOMINANT_VALUE_CHANGED:
            return "BAD OUT OF RANGE DOMINANT VALUE CHANGED";
        case Code::BAD_EDITED_OUT_OF_RANGE_DOMINANT_VALUE_CHANGED:
            return "BAD EDITED OUT OF RANGE DOMINANT VALUE CHANGED";
        case Code::BAD_OUT_OF_RANGE_DOMINANT_VALUE_CHANGED_DEPENDENT_VALUE_CHANGED:
            return "BAD OUT OF RANGE DOMINANT VALUE CHANGED DEPENDENT VALUE CHANGED";
        case Code::BAD_EDITED_OUT_OF_RANGE_DOMINANT_VALUE_CHANGED_DEPENDENT_VALUE_CHANGED:
            return "BAD EDITED OUT OF RANGE DOMINANT VALUE CHANGED DEPENDENT VALUE CHANGED";
        case Code::GOOD_COMMUNICATION_EVENT:
            return "GOOD COMMUNICATION EVENT";
        case Code::GOOD_SHUTDOWN_EVENT:
            return "GOOD SHUTDOWN EVENT";
        case Code::GOOD_CALL_AGAIN:
            return "GOOD CALL AGAIN";
        case Code::GOOD_NON_CRITICAL_TIMEOUT:
            return "GOOD NON CRITICAL TIMEOUT";
        case Code::BAD_INVALID_ARGUMENT:
            return "BAD INVALID ARGUMENT";
        case Code::BAD_CONNECTION_REJECTED:
            return "BAD CONNECTION REJECTED";
        case Code::BAD_DISCONNECT:
            return "BAD DISCONNECT";
        case Code::BAD_CONNECTION_CLOSED:
            return "BAD CONNECTION CLOSED";
        case Code::BAD_INVALID_STATE:
            return "BAD INVALID STATE";
        case Code::BAD_END_OF_STREAM:
            return "BAD END OF STREAM";
        case Code::BAD_NO_DATA_AVAILABLE:
            return "BAD NO DATA AVAILABLE";
        case Code::BAD_WAITING_FOR_RESPONSE:
            return "BAD WAITING FOR RESPONSE";
        case Code::BAD_OPERATION_ABANDONED:
            return "BAD OPERATION ABANDONED";
        case Code::BAD_EXPECTED_STREAM_TO_BLOCK:
            return "BAD EXPECTED STREAM TO BLOCK";
        case Code::BAD_WOULD_BLOCK:
            return "BAD WOULD BLOCK";
        case Code::BAD_SYNTAX_ERROR:
            return "BAD SYNTAX ERROR";
        case Code::BAD_MAX_CONNECTIONS_REACHED:
            return "BAD MAX CONNECTIONS REACHED";
        default:
            assert(false);
        }
    }


    std::string Status::ToString() const
    {
        return std::string(c_str());
    }


    Status::Code Status::ToCode() const
    {
        return mCode;
    }
}