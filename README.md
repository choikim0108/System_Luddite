# System:Luddite 🔫⏱️
![image](<img width="1918" height="1079" alt="Image" src="https://github.com/user-attachments/assets/0420755a-4a24-453b-bbcc-bb6012424ab2" />)

> **"Time moves only when you move."**
> A Superhot-inspired FPS with Roguelite progression elements, built with Unreal Engine 5.

![Unreal Engine 5](https://img.shields.io/badge/Unreal_Engine-5.3+-black?style=for-the-badge&logo=unrealengine)
![C++](https://img.shields.io/badge/Language-C++_%26_Blueprints-blue?style=for-the-badge&logo=cplusplus)
![Genre](https://img.shields.io/badge/Genre-FPS_%2F_Roguelite-red?style=for-the-badge)

## 🎮 Project Overview (프로젝트 개요)

이 프로젝트는 **"시간이 플레이어의 움직임에 종속되어 흐르는"** 독특한 메커니즘을 핵심으로 하는 FPS 게임입니다.
기존의 시간 조작 액션에 **로그라이크(Roguelite)** 성장 요소를 결합하여, 웨이브를 클리어할 때마다 다양한 **증강(Augment)** 카드를 선택해 무기를 강화하고 전략적인 전투를 펼칠 수 있습니다.

**개발 기간:** 2025.11 ~ 2025.12 (게임프로그래밍 기말 프로젝트)

**엔진 버전:** Unreal Engine 5.6.1

---

## ✨ Key Features (핵심 기능)

### 1. Time Manipulation System (시간 조작)
- 플레이어의 입력(이동, 사격)이 없을 때 **Global Time Dilation**이 극도로 느려짐.
- 탄환을 보고 피하는 **Superhot** 스타일의 액션 구현.

### 2. Roguelite Progression (로그라이크 성장)
- **Random Augment Selection:** 충분한 양의 XP 획득 시 3개의 랜덤 증강 카드 제시.
- **Infinite Stacking:** 동일한 증강 획득 시 효과 및 수치 무한 중첩 (예: 관통 횟수 증가, 연사 속도 등).
- **Dynamic HUD:** 획득한 증강 아이콘이 HUD에 실시간으로 스택(x2, x3)과 함께 표시됨.

### 3. Combat & Physics (전투 및 물리)
- **Weapon Variety:** 권총(기본 무기), 샷건(Multi-pellet) 구현.
- **Advanced Ballistics:**
  - **Ricochet (도탄):** 벽에 맞을 때 물리 각도 계산을 통해 튕겨 나감.
  - **Pierce (관통):** 적을 뚫고 지나가며, 뒤에 있는 적까지 타격 가능 (Collision Response 동적 제어).
- **Ragdoll Physics:** 적 처치 시 물리 시뮬레이션으로 전환되어 타격 방향으로 날아가는 쾌감 구현.

---

## 🛠️ Technical Deep Dive (기술적 구현 사항)

이 프로젝트는 **C++을 기반으로 핵심 로직을 설계**하고, **Blueprints로 시각적 요소와 UI를 구현**하는 하이브리드 구조를 채택했습니다.

### 🏗️ System Architecture
- **Data-Driven Design:** `UDataAsset`을 활용하여 무기(`SSWeaponData`)와 증강(`SSAugmentData`) 데이터를 관리. 코드 수정 없이 에디터에서 손쉽게 밸런싱 및 아이템 추가 가능.
- **Interface Pattern:** `ISSDamageInterface`를 통해 플레이어, 적, 파괴 가능한 오브젝트 간의 통일된 데미지 처리 구조 확립.

### 🔧 Key Challenges & Solutions
- **관통(Pierce)과 충돌 처리의 딜레마:**
  - *문제:* 관통 탄환이 적 내부에서 물리 충돌(Block)을 일으켜 멈추거나, 다중 히트 판정이 나는 문제 발생.
  - *해결:* 적(`Pawn`) 채널에 대해 `Overlap` 이벤트를 사용하고, C++에서 `HitActors` 배열을 통해 중복 피격을 방지. 벽(`Block`)과 적(`Overlap`)의 처리 로직을 분리하여 해결.
- **오디오 채널 최적화 (Audio Concurrency):**
  - *문제:* 샷건 발사 시 수십 개의 펠릿이 동시에 벽에 박히며 BGM이 끊기는 현상 (Max Channel 초과).
  - *해결:* `Sound Concurrency` 설정을 통해 동시 재생 사운드 수를 제한하고, BGM의 우선순위(Priority)를 최상으로 높여 오디오 리소스 관리.
- **연사 속도(Fire Rate)의 실시간 반영:**
  - *문제:* 단순 Loop 타이머 사용 시, 게임 도중 증강을 획득해도 연사 속도가 즉시 빨라지지 않음.
  - *해결:* 재귀적 타이머(Recursive Timer) 구조로 변경하여, 매 발사 시마다 현재의 `Multiplier`를 계산해 다음 발사 시간을 동적으로 예약하도록 개선.

---

## 💻 Controls (조작법)

| Key | Action |
| :---: | :--- |
| **W, A, S, D** | 이동 (시간 흐름 가속) |
| **Space** | 점프 (시간 흐름 가속) |
| **L-Click** | 사격 |
| **Esc, P** | 일시정지 메뉴 |
