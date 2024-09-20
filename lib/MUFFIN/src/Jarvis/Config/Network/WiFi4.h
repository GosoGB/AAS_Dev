/**
 * @file WiFi4.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Wi-Fi 인터페이스 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-09-03
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
#include <IPAddress.h>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



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
        Status SetDHCP(const bool enableDHCP);
        Status SetStaticIPv4(const std::string& staticIPv4);
        Status SetSubnet(const std::string& subnetmask);
        Status SetGateway(const std::string& gateway);
        Status SetDNS1(const std::string& dns1);
        Status SetDNS2(const std::string& dns2);
        Status SetSSID(const std::string& ssid);
        Status SetPSK(const std::string& psk) ;
        Status SetAuthMode(const std::string& auth);
        Status SetEAP(const bool& eap);
        Status SetEapID(const std::string& eapID);
        Status SetEapUserName(const std::string& eapUserName);
        Status SetEapPassword(const std::string& eapPassword);
        Status SetEapCaCertificate(const std::string& eapCaCert);
        Status SetEapClientCertificate(const std::string& eapClientCert);
        Status SetEapClientKey(const std::string& eapClientKey);

    public:
        bool GetDHCP() const;
        const IPAddress& GetStaticIPv4() const;
        const IPAddress& GetSubnet() const;
        const IPAddress& GetGateway() const;
        const IPAddress& GetDNS1() const;
        const IPAddress& GetDNS2() const;
        const std::string& GetSSID() const;
        const std::string& GetPSK() const;
        wifi_auth_mode_t GetAuthMode() const;
        bool GetEAP() const;
        const std::string& GetEapID() const;
        const std::string& GetEapUserName() const;
        const std::string& GetEapPassword() const;
        const std::string& GetEapCaCertificate() const;
        const std::string& GetEapClientCertificate() const;
        const std::string& GetEapClientKey() const;
    private:
        bool mEnableDHCP = true;
        IPAddress mStaticIPv4;
        IPAddress mSubnetmask;
        IPAddress mGateway;
        IPAddress mDNS1;
        IPAddress mDNS2;
        std::string mSSID;
        std::string mPSK;
        wifi_auth_mode_t mAuthMode;
        bool mEnableEAP = false;
        std::string mEapID;
        std::string mEapUserName;
        std::string mEapPassword;
        std::string mEapCaCertificate;
        std::string mEapClientCertificate;
        std::string mEapClientKey;
    };    
}}}