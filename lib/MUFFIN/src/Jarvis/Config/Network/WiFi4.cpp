/**
 * @file WiFi4.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Wi-Fi 인터페이스 설정 정보를 관리하는 클래스를 정의합니다.
 * 
 * @date 2024-09-03
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Logger/Logger.h"
#include "WiFi4.h"



namespace muffin { namespace jarvis { namespace config {

    WiFi4::WiFi4()
        : Base("wifi")
    {
        LOG_DEBUG(logger, "Constructed at address: %p", this);
    }

    WiFi4::~WiFi4()
    {
        LOG_DEBUG(logger, "Destroyed at address: %p", this);
    }

    void WiFi4::operator=(const WiFi4& obj)
    {
        mEnableDHCP   = obj.mEnableDHCP;
        mStaticIPv4   = obj.mStaticIPv4;
        mSubnetmask   = obj.mSubnetmask;
        mGateway      = obj.mGateway;
        mDNS1         = obj.mDNS1;
        mDNS2         = obj.mDNS2;
        mSSID         = obj.mSSID;
        mPSK          = obj.mPSK;
        mAuthMode     = obj.mAuthMode;
        mEnableEAP    = obj.mEnableEAP;
        mEapID        = obj.mEapID;
        mEapUserName  = obj.mEapUserName;
        mEapPassword  = obj.mEapPassword;
    }

    bool WiFi4::operator==(const WiFi4& obj) const
    {
        return (
            mEnableDHCP   == obj.mEnableDHCP  &&
            mStaticIPv4   == obj.mStaticIPv4  &&
            mSubnetmask   == obj.mSubnetmask  &&
            mGateway      == obj.mGateway     &&
            mDNS1         == obj.mDNS1        &&
            mDNS2         == obj.mDNS2        &&
            mSSID         == obj.mSSID        &&
            mPSK          == obj.mPSK         &&
            mAuthMode     == obj.mAuthMode    &&
            mEnableEAP    == obj.mEnableEAP   &&
            mEapID        == obj.mEapID       &&
            mEapUserName  == obj.mEapUserName &&
            mEapPassword  == obj.mEapPassword
        );
    }

    bool WiFi4::operator!=(const WiFi4& obj) const
    {
        return !(*this == obj);
    }

    Status WiFi4::SetDHCP(const bool enableDHCP)
    {
        mEnableDHCP = enableDHCP;
        if (mEnableDHCP == enableDHCP)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status WiFi4::SetStaticIPv4(const std::string& staticIPv4)
    {
        if (mStaticIPv4.fromString(staticIPv4.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status WiFi4::SetSubnet(const std::string& subnetmask)
    {
        if (mSubnetmask.fromString(subnetmask.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status WiFi4::SetGateway(const std::string& gateway)
    {
        if (mGateway.fromString(gateway.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status WiFi4::SetDNS1(const std::string& dns1)
    {
        if (mDNS1.fromString(dns1.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status WiFi4::SetDNS2(const std::string& dns2)
    {
        if (mDNS2.fromString(dns2.c_str()))
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }
    }

    Status WiFi4::SetSSID(const std::string& ssid)
    {
        assert(ssid.length() < 33);

        mSSID = ssid;
        if (mSSID == ssid)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status WiFi4::SetPSK(const std::string& psk)
    {
        assert(psk.length() < 65);

        mPSK = psk;
        if (mPSK == psk)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status WiFi4::SetAuthMode(const std::string& auth)
    {
        assert(auth == "open" || auth == "wep" || auth == "wpa_psk" || auth == "wpa2_psk" || auth == "wpa_wpa2_psk" || auth == "wpa2_enterprise");

        if (auth == "open")
        {
            mAuthMode = wifi_auth_mode_t::WIFI_AUTH_OPEN;
        }
        else if (auth == "wep")
        {
            mAuthMode = wifi_auth_mode_t::WIFI_AUTH_WEP;
        }
        else if (auth == "wpa_psk")
        {
            mAuthMode = wifi_auth_mode_t::WIFI_AUTH_WPA_PSK;
        }
        else if (auth == "wpa2_psk")
        {
            mAuthMode = wifi_auth_mode_t::WIFI_AUTH_WPA2_PSK;
        }
        else if (auth == "wpa_wpa2_psk")
        {
            mAuthMode = wifi_auth_mode_t::WIFI_AUTH_WPA_WPA2_PSK;
        }
        else
        {
            mAuthMode = wifi_auth_mode_t::WIFI_AUTH_WPA2_ENTERPRISE;
        }
        return Status(Status::Code::GOOD);
    }

    Status WiFi4::SetEAP(const bool& eap)
    {
        mEnableEAP = eap;
        if (mEnableEAP == eap)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status WiFi4::SetEapID(const std::string& eapID)
    {
        assert(eapID.length() < 65);

        mEapID = eapID;
        if (mEapID == eapID)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status WiFi4::SetEapUserName(const std::string& eapUserName)
    {
        assert(eapUserName.length() < 65);

        mEapUserName = eapUserName;
        if (mEapUserName == eapUserName)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    Status WiFi4::SetEapPassword(const std::string& eapPassword)
    {
        assert(eapPassword.length() < 65);

        mEapPassword = eapPassword;
        if (mEapPassword == eapPassword)
        {
            return Status(Status::Code::GOOD_ENTRY_REPLACED);
        }
        else
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    }

    bool WiFi4::GetDHCP() const
    {
        return mEnableDHCP;
    }

    const IPAddress& WiFi4::GetStaticIPv4() const
    {
        return mStaticIPv4;
    }

    const IPAddress& WiFi4::GetSubnet() const
    {
        return mSubnetmask;
    }

    const IPAddress& WiFi4::GetGateway() const
    {
        return mGateway;
    }

    const IPAddress& WiFi4::GetDNS1() const
    {
        return mDNS1;
    }

    const IPAddress& WiFi4::GetDNS2() const
    {
        return mDNS2;
    }

    const std::string& WiFi4::GetSSID() const
    {
        return mSSID;
    }

    const std::string& WiFi4::GetPSK() const
    {
        return mPSK;
    }

    wifi_auth_mode_t WiFi4::GetAuthMode() const
    {
        return mAuthMode;
    }

    bool WiFi4::GetEAP() const
    {
        return mEnableEAP;
    }

    const std::string& WiFi4::GetEapID() const
    {
        return mEapID;
    }

    const std::string& WiFi4::GetEapUserName() const
    {
        return mEapUserName;
    }

    const std::string& WiFi4::GetEapPassword() const
    {
        return mEapPassword;
    }
}}}