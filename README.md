# Project Wings: Chaos Room

---

> **"기체를 골라 쏘고, 당신의 의지로 날아 구조물을 파괴하라!"**
> 발사 직후부터 시작되는 실시간 조종(Post-Launch Control)과 속성 상성 시스템을 결합한 3D 물리 파괴 퍼즐 게임입니다.

> **개발 상태:** Phase 4 완료 후 개발 중단 (NEONDRIFT 프로젝트로 전환). 상세 기획 및 구현 현황은 [GDD.md](GDD.md) 참조.

---

### 1. 핵심 게임 루프 (Core Game Loop)

1. **Selection Phase**: 기체 선택 UI에서 속성 기체를 시퀀스(최대 3대)로 배치하고 확정(덱 빌딩).
2. **Launch Phase**: 발사대(`AWingsLauncher`)에서 조준 및 파워를 충전하여 발사.
3. **Flight Phase (Tactical Control)**: 실시간 3축 조종(마우스 Pitch/Yaw + A/D Roll + W/S Thrust)으로 최적의 충돌 지점 탐색.
4. **Impact & Destruction**: 기체와 타겟의 **속성이 일치할 때만** Chaos 파괴 발생. 질량·속도에 비례한 충격파로 구조물 파괴.
5. **Iteration**: 확정한 시퀀스 순서대로 기체를 소진하며, 모든 목표물(Target)을 파괴하면 스테이지 클리어.

### 2. 속성 상성 시스템 (Attribute Affinity)

게임의 핵심 메카닉. 기체와 타겟의 속성(`EWingsAttribute`)이 일치해야만 파괴가 발생합니다.

| 속성 | 설명 |
| :--- | :--- |
| `Stone` / `Wood` / `Grass` | 동일 속성 타겟만 파괴 가능 |
| `Universal` | 만능/폭탄 속성 — 모든 속성 타겟 파괴 가능 |
| `None` | 속성 없음 (기본값) |

- 속성 일치 시: `Kinematic → Dynamic` 전환 후 Chaos 파괴 + 파편 비산.
- 속성 불일치 시: 물리 충돌만 발생, `SetDynamicDamageThreshold`로 파괴 차단.

### 3. 스테이지 구상 (Stage Design — 미구현)

> 아래는 초기 구상안이며, Phase 5(레벨 디자인)는 개발 중단으로 착수되지 않았습니다. 현재는 `TestMap` / `Stage1` 프로토타입만 존재합니다.

- **Stage 1 — 비행 적응**: 탁 트인 공간의 단일 블록 타워. 발사 후 실시간 조종 개념 학습.
- **Stage 2 — 거리와 정확도**: 떨어진 지점에 배치된 복수 목표. 장거리 비행 중 미세 조정 연습.
- **Stage 3 — 정밀 기동**: 좁은 틈새·터널 지형 끝의 목표물. 경로 추종 정밀 조종.
- **Stage 4+ — 속성 전략**: 서로 다른 속성의 타겟 혼합 배치. 시퀀스에 어떤 속성 기체를 어떤 순서로 넣을지 전략적 판단 요구.

### 4. 기술 스택 (Technical Specifications)

- **Engine**: Unreal Engine 5.6 (C++ 중심)
- **Physics**: Chaos Physics (Geometry Collection, Field System)
- **Input**: Enhanced Input (State-based Context Swapping)
- **Architecture**: Data-Driven Design (`UWingsFlightData`, `UWingsDestructionData` 기반 수치 관리)

---
*이 프로젝트는 UE 5.6의 물리 파괴 기능과 속성 상성 기반 조종 액션을 실험한 미니 프로젝트입니다.*
