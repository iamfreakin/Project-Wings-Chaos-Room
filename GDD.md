# Project Wings: Chaos Room — 최종 기획서 및 구현 문서

> **상태:** 개발 중단 (NEONDRIFT 프로젝트로 전환)  
> **엔진:** Unreal Engine 5.6 (C++)  
> **마지막 빌드:** Phase 4 완료 (기체 선택 덱 빌딩 시스템까지 구현)

---

## 1. 게임 개요

| 항목 | 내용 |
|---|---|
| 장르 | 물리 기반 항공 파괴 퍼즐 액션 |
| 플랫폼 | PC |
| 핵심 루프 | 기체 선택 → 발사 → 비행 → 충돌 파괴 → 반복 |
| 승리 조건 | 제한된 기체 수 안에 스테이지의 모든 목표물(Target) 파괴 |
| 핵심 차별점 | **속성 상성 시스템** — 기체와 목표물의 속성이 일치해야만 파괴 발생 |

### 컨셉 한 줄 요약
> *"Angry Birds를 3D 공중전으로 — 속성을 맞춰 쏴라."*

---

## 2. 핵심 시스템 기획

### A. 속성(Attribute) 시스템

게임의 모든 전략적 깊이를 만드는 핵심 메카닉.

```
EWingsAttribute
├── None       — 속성 없음 (기본값)
├── Stone      — 돌 속성
├── Wood       — 나무 속성
├── Grass      — 풀 속성
└── Universal  — 만능/폭탄 속성 (모든 타겟 파괴 가능)
```

**상성 규칙:**
- 기체 속성 == 타겟 속성 → 파괴 발생
- 기체 속성 != 타겟 속성 → 물리 충돌만 발생, 파괴 없음
- `Universal` 속성 기체 → 모든 속성 타겟 파괴 가능 (단, 강력한 폭발 대신 범용성)

### B. 비행 시스템 (The Flight)

물리 기반 RigidBody 비행. 헬기나 전투기처럼 조종하는 것이 아니라, **발사체처럼 탄도 궤적 + 보조 조종**으로 목표를 겨냥하는 방식.

**조작 체계:**
| 입력 | 동작 |
|---|---|
| W/S | 추진력(Thrust) 증감 |
| 마우스 | Pitch(상하) / Yaw(좌우) |
| A/D | Roll(기울기) |
| 우클릭 홀드 | 자유 시점(Free Look) |
| R | 추락 후 발사대로 복귀 |

**물리 특성:**
- **추진력(Thrust):** 전방 벡터 기반 힘 주입, 단계적 증감
- **속도 정렬(Velocity Alignment):** 속도 방향과 기수 방향을 자동 정렬하여 미끄러짐 보정
- **가짜 양력(Fake Lift):** 속도에 비례한 중력 상쇄로 고속 수평 비행 가능
- **뱅크-투-턴(Bank to Turn):** 롤 입력 시 자동 선회 토크 부여
- **자동 수평(Auto Leveling):** 입력 없을 때 수평 자동 복원 (토글 가능)

**카메라:**
- 속도에 비례한 동적 FOV (90°~110°)
- 속도에 비례한 SpringArm 거리 변화 (800~1200)
- 충돌 후 **사망 카메라(Death Cam):** 카메라 분리, 관찰 거리 확대, 수평 자동 보정

### C. 파괴 시스템 (The Destruction)

언리얼 엔진의 **Chaos Physics + Geometry Collection** 기반.

**파괴 트리거 조건:**
1. 기체 속도 ≥ `MinBreakSpeedKmh` (데이터 에셋에서 설정)
2. 기체 속성 == 타겟 속성 (또는 Universal)

**파괴 시퀀스:**
```
충돌 감지 (OnMeshHit)
→ 속성 판정 (Attribute Match)
→ Kinematic → Dynamic 상태 전환 (고정 해제)
→ LinearImpulse 필드 주입 (파편 비산)
→ RemovalDuration 후 파편 제거 (최적화)
```

**속성 불일치 시:** `SetDynamicDamageThreshold`로 파괴 임계값을 극도로 높여 물리 충돌은 일어나되 Chaos 파괴는 차단.

### D. 발사 시스템 (The Launcher)

Angry Birds 식 충전 발사. 게임의 주요 조작 흐름의 시작점.

**발사 시퀀스:**
```
발사대 조종 (AWingsLauncher)
→ 조준 (마우스)
→ 충전 (스페이스 홀드) — 게이지 0~100%, 카메라 줌인
→ 발사 (스페이스 릴리즈) — 기체 스폰 + 초기 속력 부여
→ AWingsPlayerController가 Pawn 교체 (기체로 조종권 이전)
→ 비행 조종
→ 충돌/추락 → R키로 발사대 복귀
```

**발사대 특성:**
- 절차적(Procedural) 카메라 흔들림 (충전량 비례)
- 궤적 예측 라인 표시 (`DrawDebugSphere` 기반)
- 충전량에 따른 카메라 줌인 연출

### E. 게임 플로우 (The Flow)

**덱 빌딩 + 시퀀스 발사** 구조.

```
[기체 선택 UI]
플레이어가 3개의 슬롯에 속성 기체를 직접 배치
    ↓ ConfirmSequence()
[발사 단계]
선택한 순서대로 기체 발사 가능
    ↓ (기체 소진 or 목표 전멸)
[결과 화면]
Win: RemainingTargets == 0
Loss: 기체 소진 + 남은 타겟 존재
```

**승패 엣지 케이스 처리:** 마지막 기체와 마지막 타겟이 동시에 처리될 경우를 대비한 `LossCheckTimerHandle` 유예 타이머 적용.

---

## 3. 데이터 에셋 설계

모든 수치는 에디터에서 조절 가능한 **Data Asset** 기반으로 외부화.

### UWingsFlightData

| 카테고리 | 파라미터 | 설명 |
|---|---|---|
| Physics | PawnMass | 기체 질량 (파괴력 계산 핵심) |
| Sensitivity | PitchSensitivity, YawSensitivity, RollSensitivity | 입력 감도 |
| Physics | VelocityAlignmentSpeed | 미끄러짐 보정 강도 |
| Physics | LiftForceMultiplier | 가짜 양력 계수 |
| Physics | AutoLevelingSpeed | 자동 수평 복원 속도 |
| Physics | BankToTurnAmount | 뱅크-투-턴 강도 |
| Physics | FlightSideMoveForce | 롤 시 측면 이동력 |
| Thrust | MaxForwardThrust, ThrustStep | 최대 추진력 / 조절 단위 |
| Camera | MinFOV / MaxFOV (90~110) | 속도 기반 동적 FOV |
| Camera | MinArmLength / MaxArmLength (800~1200) | 속도 기반 카메라 거리 |
| Camera | DynamicCameraSpeedThreshold | FOV 최대치 도달 속도 |
| Camera | CameraLagSpeed, CameraRotationLagSpeed | 카메라 지연 속도 |
| Camera | FreeLookSensitivity | 자유 시점 마우스 감도 |
| Camera.Death | DeathCamDistance, DeathCamHeight | 사망 카메라 거리/높이 |
| Camera.Death | DeathCamInterpSpeed | 사망 카메라 전환 속도 |

### UWingsDestructionData

| 파라미터 | 설명 |
|---|---|
| RemovalDuration | 파편 유지 시간 (초) |
| MinBreakSpeedKmh | 파괴 최소 속도 조건 |
| ExplosionForce | 파편 비산력 |

---

## 4. 클래스 아키텍처

```
AWingsGameMode          — 게임 규칙, 시퀀스, 승패 판정
AWingsGameState         — 전역 환경 (풍향 벡터)
AWingsPlayerController  — 조종 대상 전환 (Launcher ↔ Pawn)
AWingsHUD               — UI 위젯 생성 및 이벤트 수신

AWingsLauncher (APawn)  — 발사대, 충전 발사, 궤적 예측
AWingsPawnBase (APawn)  — 비행 기체, 물리 비행, 충돌 파괴 트리거

AWingsDestructibleActor — 파괴 가능 환경 요소 (GeometryCollectionActor)

UWingsFlightData        — 비행 수치 데이터 에셋
UWingsDestructionData   — 파괴 수치 데이터 에셋
UWingsInputConfigData   — 입력 액션 데이터 에셋

UWingsUserWidget        — UI 공통 베이스 (데이터 바인딩 헬퍼)
AWingsHUD               — 위젯 인스턴스 관리

EWingsAttribute         — 전역 속성 열거형 (None/Stone/Wood/Grass/Universal)
EWingsPawnState         — 기체 상태 (Flying/Crashed)
```

---

## 5. UI 시스템

모든 UI 위젯은 `UWingsUserWidget` 상속. BlueprintPure 함수로 데이터 바인딩 제공.

| 위젯 | 표시 시점 | 주요 정보 |
|---|---|---|
| 기체 선택 UI (SelectionWidget) | 시퀀스 미확정 시 | 속성 버튼, 선택 슬롯, Confirm |
| 메인 HUD (MainHUDWidget) | 비행/발사 중 | 속도계, 충전 게이지, 남은 기체 수, 남은 타겟 수 |
| 승리 화면 (VictoryWidget) | 게임 클리어 | 다시 시작 / 다음 스테이지 |
| 게임 오버 화면 (GameOverWidget) | 기체 소진 | 다시 시작 / 종료 |

**상태 기반 가시성 로직 (`GetXxxVisibility`):**
- 발사대 조종 중 → 조준/충전 게이지만 표시
- 비행 중 → 속도계/연료 게이지만 표시
- 추락 후 → "R키 복귀 안내" 표시
- 게임 종료 → 결과 화면, 마우스 커서 활성화

---

## 6. 구현 현황

| Phase | 목표 | 상태 |
|---|---|---|
| Phase 1 | 기본 비행 물리 및 Enhanced Input 체계 | ✅ 완료 |
| Phase 2 | Chaos 파괴 시스템 및 필드 로직 | ✅ 완료 |
| Phase 3 | 속성 상성 시스템 및 게임 모드 승패 판정 | ✅ 완료 |
| Phase 4 | 기체 선택 UI (덱 빌딩) 및 발사 시퀀스 | ✅ 완료 |
| Phase 5 | 정식 스테이지 디자인 및 밸런싱 | ❌ 미착수 (개발 중단) |

### 구현 완료 세부 항목

**비행:**
- [x] 물리 기반 3축 비행 (Pitch/Yaw/Roll 토크)
- [x] 추진력(Thrust) 단계 조절
- [x] 속도 정렬 (Velocity Alignment)
- [x] 가짜 양력 (Fake Lift)
- [x] 뱅크-투-턴 (Bank to Turn)
- [x] 자동 수평 복원 (Auto Leveling, 토글)
- [x] 자유 시점 (Free Look, 우클릭)
- [x] 동적 FOV + 카메라 거리 (속도 연동)
- [x] 사망 카메라 (충돌 후 분리, 수평 보정, 궤도 관찰)

**파괴:**
- [x] Chaos Physics + Geometry Collection 연동
- [x] 속성 일치 판정 후 Kinematic→Dynamic 전환
- [x] LinearImpulse 필드로 파편 비산
- [x] 속성 불일치 시 파괴 차단 (임계값 조작)
- [x] Universal 속성 (모든 타겟 파괴)
- [x] 파편 RemovalDuration 후 자동 제거

**발사대:**
- [x] 충전 발사 (스페이스 홀드/릴리즈)
- [x] 궤적 예측 라인
- [x] 충전량 비례 카메라 줌인
- [x] 절차적 카메라 흔들림 (Shake)

**게임 모드:**
- [x] 기체 선택 덱 빌딩 (최대 3슬롯)
- [x] 시퀀스 순서대로 기체 발사
- [x] 타겟 카운팅 및 승리 조건
- [x] 기체 소진 시 패배 조건
- [x] 추락 후 R키 발사대 복귀
- [x] 레이스 컨디션 방지 유예 타이머

**UI:**
- [x] 기체 선택 화면
- [x] 속도계 / 충전 게이지 / 타겟 카운터
- [x] 승리/패배 결과 화면
- [x] 상태 기반 가시성 자동 전환

### 미구현 항목 (개발 중단으로 미착수)

- [ ] 정식 스테이지 레벨 디자인 (TestMap / Stage1 에셋만 존재)
- [ ] 기체 외형 에셋 (메쉬 미할당 상태)
- [ ] 타겟 오브젝트 다양화 (현재 큐브 기반 프로토타입)
- [ ] BGM / SFX
- [ ] 메인 메뉴

---

## 7. 폴더 구조

```
ProjectWings/
├── Source/ProjectWings/
│   ├── Public/Core/          WingsGameMode, WingsGameState, WingsPlayerController
│   ├── Public/Pawn/          WingsPawnBase
│   ├── Public/Launcher/      WingsLauncher
│   ├── Public/Environment/   WingsDestructibleActor
│   ├── Public/Data/          WingsFlightData, WingsDestructionData, WingsInputConfigData
│   └── Public/UI/            WingsHUD, WingsUserWidget
│
└── Content/
    ├── Blueprints/           BP_Blue_Cube (파괴 가능 큐브 프로토타입)
    ├── Data/                 DA_WingsFlight_Default (비행 데이터 에셋)
    ├── Maps/                 TestMap, Stage1 (프로토타입 레벨)
    ├── GeometryCollections/  파괴용 Chaos 에셋
    └── UI/                   위젯 블루프린트
```

---

## 8. 개발 노트

- **UE 5.6 대응:** Chaos Physics 헤더 경로 변경 및 모듈 의존성 수동 추가 필요
- **Kinematic 전환 타이밍:** `BeginPlay`에서 모든 Geometry Collection을 Kinematic으로 강제 초기화하지 않으면 에디터 시작 시 물리 활성화로 오브젝트 붕괴 발생
- **레이스 컨디션:** 마지막 기체 충돌 프레임에 마지막 타겟도 파괴되면 패배 판정이 먼저 뜨는 문제 → 1프레임 유예 타이머(`LossCheckTimerHandle`)로 해결
- **속성 불일치 차단:** `SetDamageThreshold` 대신 `SetDynamicDamageThreshold`로 런타임 임계값 조작 방식 채택 (UE 5.6 API 변경 대응)
