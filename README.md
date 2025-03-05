# 릴리즈 노트 Mar 4, 2025
## MUFFIN 프레임워크 Ver.1.3.0 개발 완료
### 신규 기능
  - 인터페이스
    - [MODLINK-T3] LINK1, LINK2 설정 및 RS-485 인터페이스 개발
    - [MODLINK-T2] 서비스 네트워크 Ethernet 추가
  - CLI 모드
    - [MODLINK-T2,T3] 부팅 후 5초 내 Enter 키 입력 시 CLI 모드로 진입, 서비스 네트워크를 설정 기능 추가
  - FOTA 청크 기능 추가
    - 서버에 청크 단위로 저장된 펌웨어를 업데이트하며 실시간으로 업데이트 현황을 서버로 전송 하는 기능 추가
  - STATUS 전송 기능 추가
    - 서버에서 디바이스 설정값 요청시 전송 기능 추가
    - 1시간 마다 혹은 특정 이벤트가 발생 시 서버로 디바이스 STATUS 정보 전송
### 버그 수정
  - [LTE 사용 시] MQTT 브로커 재연결시 NTP 시간 동기화 버그 수정
### 기능 개선
  - [MODBUS TCP] SlaveID 추가, Multi Slave 설정 시 여러개의 Client를 생성해 리소스 관리
  - [FOTA] 부팅 시 서버에 자동 업데이트 버전을 확인해 자동으로 진행되는 기능 삭제
# <br><br><br>
# 릴리즈 노트 Jan 8, 2025
## MUFFIN 프레임워크 Ver.1.2.1 개발 완료
### 버그 수정
  - MFM과 FOTA https 요청을 동시에 처리하는 로직을 수정함
# <br><br><br>
# 릴리즈 노트 Jan 6, 2025
## MUFFIN 프레임워크 Ver.1.2.0 개발 완료
### 신규 기능
  - 인터페이스
    - [MODLINK-T2] LINK1, LINK2 설정 및 RS-485 인터페이스 개발
   
  - 기능 추가
    - [MODLINK-T2] Modbus TCP 멀티 슬레이브 기능 지원
    - [MODLINK-T2] Modbus RTU 멀티 슬레이브 기능 지원
    - [MODLINK-L] Modbus RTU 멀티 슬레이브 기능 지원

### 기능 제약
  - Ver.1.2.0에서는 단일 레지스터로만 원격제어가 가능합니다.
  - Ver.1.2.0에서는 MFM 설정 값을 수신할 경우에는 디바이스를 리셋합니다.
  - [MODLINK-L] : 최대 설정 노드를 50개로 제안합니다. 
  - [MODLINK-T2] : 최대 노드를 35개로 제안합니다.
# <br><br><br>
# 릴리즈 노트 Nov 26, 2024
## MUFFIN 프레임워크 Ver.1.1.0 개발 완료
### 신규 기능
  - 네트워크
    - [MODLINK-T2] Ethernet 통신 인터페이스 개발
  - 프로토콜
    - [MODLINK-T2] Modbus TCP 통신 프로토콜 개발

### 기능 제약
  - [MODLINK-T2] Ver.1.1.0에서는 Modbus RTU 통신 프로토콜을 지원하지 않습니다.
  - [MODLINK-T2] Ver.1.1.0에서는 단일 슬레이브와의 통신만을 지원합니다.
  - [MODLINK-T2] Ver.1.1.0에서는 단일 레지스터로만 원격제어가 가능합니다.
  - [MODLINK-T2] Ver.1.1.0에서는 Modbus TCP 또는 Ethernet 설정을 수신할 경우에는 디바이스를 리셋합니다.

### 알려진 버그
  - MFM 설정이 끝났음에도 불구하고 "unavailable due to jarvis task being already running or blocked" 코드가 출력되는 버그가 보고됨
  - 설정값 초기화(factory reset) 명령 실행 이후에 디바이스 동작 속도가 눈의 띄게 느려지는 버그가 보고됨
# <br><br><br>
# 릴리즈 노트 Nov 20, 2024
## MUFFIN 프레임워크 Ver.1.0.0 개발 완료
### 신규 기능
  - 네트워크
    - LTE Cat.M1 통신 기능 개발
  - 프로토콜
    - MQTT(S)
    - HTTP(S)
    - Modbus RTU
  - 스토리지
    - EspFS
    - CatFS
  - JARVIS
    - Validator
    - Config
  - 정보 모델
    - Base Node
    - Variable Node
       - 기계 데이터 변환
       - 이벤트 트리거링
    - Built-in Data Types
  - OTA
### 기능 제약
  - Ver.1.0.0에서는 설정 가능한 Node의 개수가 50개로 제한됨
    - 추후 버전에서 설정 가능 개수를 늘릴 예정임
  - Ver.1.0.0에서는 LTE 모듈을 사용해야 하며, ESP32 칩셋만 OTA 가능함
    - 추후 버전에서 Wi-Fi, Ethernet으로도 OTA 기능을 개발할 예정임
    - 추후 버전에서 ATmega2560 칩셋 대상으로 OTA 기능을 개발할 예정임
  - Ver.1.0.0에서는 Modbus 프로토콜만 지원되며, 단일 슬레이브만 지원함
    - 또한 원격제어 명령의 경우, 단일 레지스터로만 가능함
    - 추후 버전에서 Modbus TCP 프로토콜을 지원할 예정임

### 알려진 버그
  - OTA를 연속으로 시도할 때 실패하는 현상이 보고됨
     - 추후 버전에서 수정할 예정임
