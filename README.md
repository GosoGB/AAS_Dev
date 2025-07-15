# 📝 AAS for MUFFIN (Asset Administration Shell)

### 📌 목적
MODLINK에 AAS 서비스 기능 구현

---

### ⏳ 기간
AAS 개발은 2025년도부터 2개년도에 걸쳐 단계적으로 추진  
- **1차년도:** 소진공 스마트제조 지원사업 수행에 필요한 범위 내에서 개발  
- **2차년도:** AAS-Ready 디바이스로 제품화 및 서비스화 가능한 수준을 목표로 고도화

---

### 📦 범위

- **대상 디바이스:**  
  - 🚩 MT11 또는 MT11 상위 기종

- **AAS 표준 사양:**  
  - **AAS 메타모델:** Ver. 3.0  
  - **AAS API:** Ver. 3.0

- **🛠️ AAS 서비스**
    - **Registry**
        - AAS Descriptor cardinality: `0..1`
        - Submodel cardinality: `0..2`
            - OperationalData, TinyML
            - 2차년도에 확장 예정
    - **Repository**
        - 인증: **Basic Authentication** 방식만 지원
        - 인가: 지원하지 않음
        - 암호: **TLS with private cert**

---

### 📚 참조 자료

- [AAS 시스템 설계 및 개발안 - Google Slides](https://docs.google.com/presentation/d/1NWkLF7XYZ8OllXszv8sXUnAlGn4tBXhNEJDuJkfJBJI/edit?slide=id.g36bb4e79687_0_0)
- [[소진공] 스마트제조 지원사업 추가과업 - Monday Item](https://edgecross.monday.com/boards/3772214321/pulses/8676370651)