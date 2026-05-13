# Project Wings: Chaos Room (개정판)

---

> **"포트리스처럼 쏘고, 당신의 의지로 날아 구조물을 파괴하라!"**  
> 일반적인 투사체 게임과 달리 발사 직후부터 시작되는 실시간 조종(Post-Launch Control)을 통해 구조물의 약점을 정밀 타격하는 3D 물리 파괴 게임입니다.

---

### 1. 핵심 게임 루프 (Core Game Loop)

1. **Launch Phase**: 발사대에서 각도 및 파워를 조절하여 발사.
2. **Flight Phase (Tactical Control)**: 실시간 3축 조종(WASD)을 통해 장애물을 회피하고 최적의 충돌 지점(약점) 탐색.
3. **Impact & Destruction**: 기체의 질량과 속도에 비례한 물리적 충격파를 발생시켜 구조물 파괴.
4. **Iteration**: 각 스테이지에 주어진 기체 수(기본 3대) 내에 목표물을 파괴하면 스테이지 클리어.

### 2. 스테이지 로드맵 (Stage Design)

#### Stage 1: 비행 적응 (Flight Initiation)
- **목표**: 비행 기초 조작 익히기.
- **구성**: 탁 트인 공간 중앙의 거대 블록 타워.
- **의도**: 발사 후 실시간 조종이 가능하다는 개념을 유저에게 각인.

#### Stage 2: 거리와 정확도 (Distance & Accuracy)
- **목표**: 장거리 비행 및 순차적 목표 제거.
- **구성**: 서로 떨어진 3개의 지점에 각각 배치된 목표 블록.
- **의도**: 연료 관리와 장거리 비행 중의 미세 조정 연습.

#### Stage 3: 정밀 기동 (Precision Maneuvering)
- **목표**: 복잡한 지형 통과 및 약점 타격.
- **구성**: 좁은 틈새, 박스 터널 등 동굴 형태의 지형 끝에 목표물 위치.
- **의도**: 벽에 충돌하지 않고 경로를 따라가는 정밀 조종의 재미 극대화.

#### Stage 4+: 특수 기체 및 기믹 (Specialized Combat)
- **목표**: 기체별 상성을 활용한 전략적 파괴.
- **특수 요소**:
    - **철갑 블록**: 높은 임계치를 가져 일반 기체로는 파괴 불가능.
    - **Heavy Plane**: 높은 질량으로 철갑 블록을 돌파하는 공성 기체.
    - **Bomb Plane**: 충돌 시 강력한 카오스 필드를 생성하는 광역 파괴 기체.

### 3. 기술 스택 (Technical Specifications)

- **Engine**: Unreal Engine 5.6.1 (C++ 중심)
- **Physics**: Chaos Physics (Geometry Collection, Field System)
- **Input**: Enhanced Input (State-based Context Swapping)
- **Architecture**: Data-Driven Design (Primary Data Asset 기반 기체/블록 스펙 관리)

---
*이 프로젝트는 UE 5.6의 최신 기능을 활용하여 물리 기반의 파괴감과 조종의 재미를 실험하는 미니 프로젝트입니다.*
