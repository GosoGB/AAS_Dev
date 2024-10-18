/**
 * @file WiFi4.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Wi-Fi 인터페이스 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @todo JARVIS 설계 문서에 WPA2 authentication method 속성을 추가로 정의해야 합니다.
 * 그러나 그에 앞서서 각 auth 모드에 따라 사용자로부터 추가로 입력을 받아야 하는 데이터가 
 * 어떤 게 있는지를 먼저 확인해야 합니다. WPA2 auth 방법에는 아래의 세 가지가 있습니다.
 *   - WPA2_AUTH_TLS, WPA2_AUTH_PEAP, WPA2_AUTH_TTLS
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <esp_wifi_types.h>
#include <IPv6Address.h>
#include <IPAddress.h>
#include <WiFiSTA.h>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis { namespace config {

    class WiFi4 : public Base
    {
    public:
        WiFi4();
        virtual ~WiFi4() override;
    public:
        WiFi4& operator=(const WiFi4& obj);
        bool operator==(const WiFi4& obj) const;
        bool operator!=(const WiFi4& obj) const;
    public:
        void SetDHCP(const bool enableDHCP);
        void SetStaticIPv4(const IPAddress& staticIPv4);
        void SetSubnetmask(const IPAddress& subnetmask);
        void SetGateway(const IPAddress& gateway);
        void SetDNS1(const IPAddress& dns1);
        void SetDNS2(const IPAddress& dns2);
        void SetSSID(const std::string& ssid);
        void SetPSK(const std::string& psk) ;
        void SetAuthMode(const wifi_auth_mode_t auth);
        void SetEAP(const bool enableEAP);
        void SetEapAuthMode(const wpa2_auth_method_t eapAuth);
        void SetEapID(const std::string& eapID);
        void SetEapUserName(const std::string& eapUserName);
        void SetEapPassword(const std::string& eapPassword);
        void SetEapCaCertificate(const std::string& eapCaCert);
        void SetEapClientCertificate(const std::string& eapClientCert);
        void SetEapClientKey(const std::string& eapClientKey);
    public:
        std::pair<Status, bool> GetDHCP() const;
        std::pair<Status, IPAddress> GetStaticIPv4() const;
        std::pair<Status, IPAddress> GetSubnetmask() const;
        std::pair<Status, IPAddress> GetGateway() const;
        std::pair<Status, IPAddress> GetDNS1() const;
        std::pair<Status, IPAddress> GetDNS2() const;
        std::pair<Status, std::string> GetSSID() const;
        std::pair<Status, std::string> GetPSK()  const;
        std::pair<Status, wifi_auth_mode_t> GetAuthMode() const;
        std::pair<Status, bool> GetEAP() const;
        std::pair<Status, wpa2_auth_method_t> GetEapAuthMode() const;
        std::pair<Status, std::string> GetEapID() const;
        std::pair<Status, std::string> GetEapUserName() const;
        std::pair<Status, std::string> GetEapPassword() const;
        std::pair<Status, std::string> GetEapCaCertificate() const;
        std::pair<Status, std::string> GetEapClientCertificate() const;
        std::pair<Status, std::string> GetEapClientKey() const;
    private:
        bool mIsEnableDhcpSet             = false;
        bool mIsStaticIPv4Set             = false;
        bool mIsSubnetmaskSet             = false;
        bool mIsGatewaySet                = false;
        bool mIsDNS1Set                   = false;
        bool mIsDNS2Set                   = false;
        bool mIsSsidSet                   = false;
        bool mIsPskSet                    = false;
        bool mIsAuthModeSet               = false;
        bool mIsEnableEapSet              = false;
        bool mIsEapAuthModeSet            = false;
        bool mIsEapIdSet                  = false;
        bool mIsEapUserNameSet            = false;
        bool mIsEapPasswordSet            = false;
        bool mIsEapCaCertificateSet       = false;
        bool mIsEapClientCertificateSet   = false;
        bool mIsEapClientKeySet           = false;
    private:
        bool mEnableDHCP;
        IPAddress mStaticIPv4;
        IPAddress mSubnetmask;
        IPAddress mGateway;
        IPAddress mDNS1;
        IPAddress mDNS2;
        std::string mSSID;
        std::string mPSK;
        wifi_auth_mode_t mAuthMode;
        bool mEnableEAP;
        wpa2_auth_method_t mEapAuthMode;
        std::string mEapID;
        std::string mEapUserName;
        std::string mEapPassword;
        std::string mEapCaCertificate;
        std::string mEapClientCertificate;
        std::string mEapClientKey;
    };    
}}}