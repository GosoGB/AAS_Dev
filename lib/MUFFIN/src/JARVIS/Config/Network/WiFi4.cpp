/**
 * @file WiFi4.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Wi-Fi 인터페이스 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <regex> 

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "WiFi4.h"



namespace muffin { namespace jarvis { namespace config {

    WiFi4::WiFi4()
        : Base(cfg_key_e::WIFI4)
    {
    }

    WiFi4::~WiFi4()
    {
    }

    WiFi4& WiFi4::operator=(const WiFi4& obj)
    {
        if (this != &obj)
        {
            mEnableDHCP             = obj.mEnableDHCP;
            mStaticIPv4             = obj.mStaticIPv4;
            mSubnetmask             = obj.mSubnetmask;
            mGateway                = obj.mGateway;
            mDNS1                   = obj.mDNS1;
            mDNS2                   = obj.mDNS2;
            mSSID                   = obj.mSSID;
            mPSK                    = obj.mPSK;
            mAuthMode               = obj.mAuthMode;
            mEnableEAP              = obj.mEnableEAP;
            mEapID                  = obj.mEapID;
            mEapUserName            = obj.mEapUserName;
            mEapPassword            = obj.mEapPassword;
            mEapCaCertificate       = obj.mEapCaCertificate;
            mEapClientCertificate   = obj.mEapClientCertificate;
            mEapClientKey           = obj.mEapClientKey;
        }
        
        return *this;
    }

    bool WiFi4::operator==(const WiFi4& obj) const
    {
        return (
            mEnableDHCP             == obj.mEnableDHCP              &&
            mStaticIPv4             == obj.mStaticIPv4              &&
            mSubnetmask             == obj.mSubnetmask              &&
            mGateway                == obj.mGateway                 &&
            mDNS1                   == obj.mDNS1                    &&
            mDNS2                   == obj.mDNS2                    &&
            mSSID                   == obj.mSSID                    &&
            mPSK                    == obj.mPSK                     &&
            mAuthMode               == obj.mAuthMode                &&
            mEnableEAP              == obj.mEnableEAP               &&
            mEapID                  == obj.mEapID                   &&
            mEapUserName            == obj.mEapUserName             &&
            mEapPassword            == obj.mEapPassword             &&
            mEapCaCertificate       == obj.mEapCaCertificate        &&
            mEapClientCertificate   == obj.mEapClientCertificate    &&
            mEapClientKey           == obj.mEapClientKey
        );
    }

    bool WiFi4::operator!=(const WiFi4& obj) const
    {
        return !(*this == obj);
    }

    void WiFi4::SetDHCP(const bool enableDHCP)
    {
        ASSERT(
            (
                mIsStaticIPv4Set == false &&
                mIsSubnetmaskSet == false &&
                mIsGatewaySet    == false &&
                mIsDNS1Set       == false &&
                mIsDNS2Set       == false
            ), "INVALID PRECONDITION: CANNOT SET STATIC IPv4 PRIOR TO DHCP"
        );

        mEnableDHCP = enableDHCP;
        mIsEnableDhcpSet = true;

        ASSERT(
            (
                mStaticIPv4 == INADDR_NONE &&
                mSubnetmask == INADDR_NONE &&
                mGateway    == INADDR_NONE &&
                mDNS1       == INADDR_NONE &&
                mDNS2       == INADDR_NONE
            ), "INVALID POSTCONDITION: IPv4 ADDRESSES MUST BE DEFAULT VALUE WHICH IS 0.0.0.0"
        );
    }

    void WiFi4::SetStaticIPv4(const IPAddress& staticIPv4)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                staticIPv4 != IPAddress(0, 0, 0, 0)        ||
                staticIPv4 != IPAddress(127, 0, 0, 1)      ||
                staticIPv4 != IPAddress(192, 0, 2, 0)      ||
                staticIPv4 != IPAddress(203, 0, 113, 0)    ||
                staticIPv4 != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mStaticIPv4 = staticIPv4;
        mIsStaticIPv4Set = true;
    }

    void WiFi4::SetSubnetmask(const IPAddress& subnetmask)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                subnetmask != IPAddress(0, 0, 0, 0)        ||
                subnetmask != IPAddress(127, 0, 0, 1)      ||
                subnetmask != IPAddress(192, 0, 2, 0)      ||
                subnetmask != IPAddress(203, 0, 113, 0)    ||
                subnetmask != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mSubnetmask = subnetmask;
        mIsSubnetmaskSet = true;
    }

    void WiFi4::SetGateway(const IPAddress& gateway)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                gateway != IPAddress(0, 0, 0, 0)        ||
                gateway != IPAddress(127, 0, 0, 1)      ||
                gateway != IPAddress(192, 0, 2, 0)      ||
                gateway != IPAddress(203, 0, 113, 0)    ||
                gateway != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mGateway = gateway;
        mIsGatewaySet = true;
    }

    void WiFi4::SetDNS1(const IPAddress& dns1)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                dns1 != IPAddress(0, 0, 0, 0)        ||
                dns1 != IPAddress(127, 0, 0, 1)      ||
                dns1 != IPAddress(192, 0, 2, 0)      ||
                dns1 != IPAddress(203, 0, 113, 0)    ||
                dns1 != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mDNS1 = dns1;
        mIsDNS1Set = true;
    }

    void WiFi4::SetDNS2(const IPAddress& dns2)
    {
        ASSERT((mIsEnableDhcpSet == true), "DHCP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableDHCP == false), "DHCP MUST BE TURNED OFF TO SET STATIC IPv4");
        ASSERT(
            (
                dns2 != IPAddress(0, 0, 0, 0)        ||
                dns2 != IPAddress(127, 0, 0, 1)      ||
                dns2 != IPAddress(192, 0, 2, 0)      ||
                dns2 != IPAddress(203, 0, 113, 0)    ||
                dns2 != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mDNS2 = dns2;
        mIsDNS2Set = true;
    }

    void WiFi4::SetSSID(const std::string& ssid)
    {
        ASSERT((ssid.length() < 33), "SSID CANNOT EXCEED 32 CHARACTERS");
        ASSERT((ssid.empty() == false), "SSID CANNOT BE AN EMPTY STRING");
        ASSERT(
            (
                [&]()
                {
                    std::regex invalidChars("[<>#%&{}\\\\\\s`~!@\\$\\^\\*\\(\\)\\[\\];:\",.]+");
                    return std::regex_search(ssid, invalidChars) == false;
                }()
            ), "INVALID CHARACTER IS INCLUDED IN THE SSID: %s", ssid.c_str()
        );

        mSSID = ssid;
        mIsSsidSet = true;
    }

    void WiFi4::SetPSK(const std::string& psk)
    {
        ASSERT((psk.length() < 65), "PSK CANNOT EXCEED 64 CHARACTERS");
        ASSERT((psk.length() >  7), "PSK CANNOT BE SHORTER THAN 8 CHARACTERS");
        ASSERT((mIsAuthModeSet == true), "AUTH MODE MUST BE SET BEFOREHAND");
        ASSERT((mAuthMode != wifi_auth_mode_t::WIFI_AUTH_OPEN), "PSK CANNOT BE SET WHEN AUTH MODE: \"open\"");
        ASSERT((mIsEnableEapSet == true), "EAP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableEAP == false), "PSK CANNOT BE SET WHEN EAP IS ENABLED");

        mPSK = psk;
        mIsPskSet = true;
    }

    void WiFi4::SetAuthMode(const wifi_auth_mode_t auth)
    {
        ASSERT((mIsEnableEapSet == true), "EAP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableEAP == false), "AUTH MODE CANNOT BE SET WHEN EAP IS ENABLED");
        ASSERT(
            (
                auth == wifi_auth_mode_t::WIFI_AUTH_OPEN      ||
                auth == wifi_auth_mode_t::WIFI_AUTH_WEP       ||
                auth == wifi_auth_mode_t::WIFI_AUTH_WPA_PSK   ||
                auth == wifi_auth_mode_t::WIFI_AUTH_WPA2_PSK  ||
                auth == wifi_auth_mode_t::WIFI_AUTH_WPA_WPA2_PSK
            ),
            "UNSUPPORTED Wi-Fi AUTH MODE: %u", static_cast<uint8_t>(auth)
        );

        mAuthMode = auth;
        mIsAuthModeSet = true;
    }

    void WiFi4::SetEAP(const bool enableEAP)
    {
        ASSERT(
            (
                mIsPskSet == false &&
                mIsAuthModeSet == false
            ), "PSK AND AUTH MODE CANNOT BE SET BEFORE EAP DOES"
        );

        mEnableEAP = enableEAP;
        mIsEnableEapSet = true;
    }

    void WiFi4::SetEapAuthMode(const wpa2_auth_method_t eapAuth)
    {
        ASSERT((mIsEnableEapSet == true), "EAP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableEAP == true), "EAP MUST BE ENABLED TO SET EAP AUTH MODE");

        mEapAuthMode = eapAuth;
        mIsEapAuthModeSet = true;

        ASSERT(
            (
                [&]()
                {
                    if (mEapAuthMode == wpa2_auth_method_t::WPA2_AUTH_TLS)
                    {
                        return
                        (
                            mIsEapClientCertificateSet &&
                            mIsEapClientKeySet
                        );
                    }
                    else
                    {
                        return true;
                    }
                }()
            ), "CLIENT CERTIFICATE AND KEY MUST BE SET BEFOREHAND TO SET EAP TLS MODE"
        );
    }

    void WiFi4::SetEapID(const std::string& eapID)
    {
        ASSERT((mIsEnableEapSet == true), "EAP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableEAP == true), "EAP MUST BE ENABLED TO SET EAP ID");

        ASSERT((mIsEapAuthModeSet == true), "EAP AUTH MODE MUST BE SET BEFOREHAND");
        ASSERT((mEapAuthMode != wpa2_auth_method_t::WPA2_AUTH_TLS), "EAP ID CANNOT BE SET WHEN AUTH MODE IS TLS");
        
        ASSERT((eapID.length() < 65), "EAP ID CANNOT EXCEED 64 CHARACTERS");
        ASSERT((eapID.empty() == false), "EAP ID CANNOT BE AN EMPTY STRING");

        mEapID = eapID;
        mIsEapIdSet = true;
    }

    void WiFi4::SetEapUserName(const std::string& eapUserName)
    {
        ASSERT((mIsEnableEapSet == true), "EAP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableEAP == true), "EAP MUST BE ENABLED TO SET EAP USERNAME");

        ASSERT((mIsEapAuthModeSet == true), "EAP AUTH MODE MUST BE SET BEFOREHAND");
        ASSERT((mEapAuthMode != wpa2_auth_method_t::WPA2_AUTH_TLS), "EAP USERNAME CANNOT BE SET WHEN AUTH MODE IS TLS");

        ASSERT((eapUserName.length() < 65), "EAP USERNAME CANNOT EXCEED 64 CHARACTERS");
        ASSERT((eapUserName.empty() == false), "EAP USERNAME CANNOT BE AN EMPTY STRING");

        mEapUserName = eapUserName;
        mIsEapUserNameSet = true;
    }

    void WiFi4::SetEapPassword(const std::string& eapPassword)
    {
        ASSERT((mIsEnableEapSet == true), "EAP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableEAP == true), "EAP MUST BE ENABLED TO SET EAP PASSWORD");

        ASSERT((mIsEapAuthModeSet == true), "EAP AUTH MODE MUST BE SET BEFOREHAND");
        ASSERT((mEapAuthMode != wpa2_auth_method_t::WPA2_AUTH_TLS), "EAP PASSWORD CANNOT BE SET WHEN AUTH MODE IS TLS");

        ASSERT((eapPassword.length() < 65), "EAP PASSWORD CANNOT EXCEED 64 CHARACTERS");
        ASSERT((eapPassword.empty() == false), "EAP PASSWORD CANNOT BE AN EMPTY STRING");

        mEapPassword = eapPassword;
        mIsEapPasswordSet = true;
    }

    void WiFi4::SetEapCaCertificate(const std::string& eapCaCert)
    {
        ASSERT((mIsEnableEapSet == true), "EAP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableEAP == true), "EAP MUST BE ENABLED TO SET EAP CA CERTIFICATE");

        mEapCaCertificate = eapCaCert;
        mIsEapCaCertificateSet = true;
    }

    void WiFi4::SetEapClientCertificate(const std::string& eapClientCert)
    {
        ASSERT((mIsEnableEapSet == true), "EAP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableEAP == true), "EAP MUST BE ENABLED TO SET EAP CLIENT CERTIFICATE");

        mEapClientCertificate = eapClientCert;
        mIsEapClientCertificateSet = true;
    }

    void WiFi4::SetEapClientKey(const std::string& eapClientKey)
    {
        ASSERT((mIsEnableEapSet == true), "EAP ENABLEMENT MUST BE SET BEFOREHAND");
        ASSERT((mEnableEAP == true), "EAP MUST BE ENABLED TO SET EAP CLIENT KEY");

        mEapClientKey = eapClientKey;
        mIsEapClientKeySet = true;
    }

    std::pair<Status, bool> WiFi4::GetDHCP() const
    {
        if (mIsEnableDhcpSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEnableDHCP);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEnableDHCP);
        }
    }

    std::pair<Status, IPAddress> WiFi4::GetStaticIPv4() const
    {
        if (mIsStaticIPv4Set)
        {
            return std::make_pair(Status(Status::Code::GOOD), mStaticIPv4);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mStaticIPv4);
        }
    }

    std::pair<Status, IPAddress> WiFi4::GetSubnetmask() const
    {
        if (mIsSubnetmaskSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mSubnetmask);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mSubnetmask);
        }
    }

    std::pair<Status, IPAddress> WiFi4::GetGateway() const
    {
        if (mIsGatewaySet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mGateway);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mGateway);
        }
    }

    std::pair<Status, IPAddress> WiFi4::GetDNS1() const
    {
        if (mIsDNS1Set)
        {
            return std::make_pair(Status(Status::Code::GOOD), mDNS1);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDNS1);
        }
    }

    std::pair<Status, IPAddress> WiFi4::GetDNS2() const
    {
        if (mIsDNS2Set)
        {
            return std::make_pair(Status(Status::Code::GOOD), mDNS2);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDNS2);
        }
    }

    std::pair<Status, std::string> WiFi4::GetSSID() const
    {
        if (mIsSsidSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mSSID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mSSID);
        }
    }

    std::pair<Status, std::string> WiFi4::GetPSK()  const
    {
        if (mIsPskSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mPSK);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mPSK);
        }
    }

    std::pair<Status, wifi_auth_mode_t> WiFi4::GetAuthMode() const
    {
        if (mIsAuthModeSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mAuthMode);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mAuthMode);
        }
    }

    std::pair<Status, bool> WiFi4::GetEAP() const
    {
        if (mIsEnableEapSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEnableEAP);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEnableEAP);
        }
    }

    std::pair<Status, wpa2_auth_method_t> WiFi4::GetEapAuthMode() const
    {
        if (mIsEapAuthModeSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEapAuthMode);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEapAuthMode);
        }
    }

    std::pair<Status, std::string> WiFi4::GetEapID() const
    {
        if (mIsEapIdSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEapID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEapID);
        }
    }

    std::pair<Status, std::string> WiFi4::GetEapUserName() const
    {
        if (mIsEapUserNameSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEapUserName);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEapUserName);
        }
    }

    std::pair<Status, std::string> WiFi4::GetEapPassword() const
    {
        if (mIsEapPasswordSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEapPassword);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEapPassword);
        }
    }

    std::pair<Status, std::string> WiFi4::GetEapCaCertificate() const
    {
        if (mIsEapCaCertificateSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEapCaCertificate);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEapCaCertificate);
        }
    }

    std::pair<Status, std::string> WiFi4::GetEapClientCertificate() const
    {
        if (mIsEapClientCertificateSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEapClientCertificate);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEapClientCertificate);
        }
    }

    std::pair<Status, std::string> WiFi4::GetEapClientKey() const
    {
        if (mIsEapClientKeySet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEapClientKey);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEapClientKey);
        }
    }
}}}