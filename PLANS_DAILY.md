# Project Wings: Daily Development Plans

## [1주차 1일차] WingsPawnBase 컴포넌트 구성 및 기초 설계 (완료)

### 1. 목표
- `AWingsPawnBase` 클래스의 물리 기반 구조 완성.
- 에디터에서 상속받은 블루프린트(`BP_WingsPawn`)를 생성하고, 실제 조종 가능한 기체로 동작하기 위한 컴포넌트 설정 수치 확정.
- 엔진 트레일(Niagara) 및 카메라 시야각 등 시각적 기초 환경 구축.

### 2. 영향 범위
- `ProjectWings/Source/ProjectWings/Public/Pawn/WingsPawnBase.h`
- `ProjectWings/Source/ProjectWings/Private/Pawn/WingsPawnBase.cpp`
- `Content/Blueprints/BP_WingsPawn.uasset` (신규 생성)

### 3. 상세 단계 (C++ Implementation)
1. **컴포넌트 선언**: `MeshComponent`, `SpringArm`, `Camera`, `EngineTrail` 컴포넌트 추가.
2. **물리 속성 설정**: 생성자에서 `LinearDamping`, `AngularDamping` 설정으로 물리 안정성 확보.
3. **카메라 시스템**: `SpringArm`의 `TargetArmLength` 및 `SocketOffset` 조정으로 비행 시야 확보.
4. **기본 스탯**: `InitialLaunchForce` 변수 추가 및 기본값 설정.

### 4. Editor Workflow (중요)
1. **블루프린트 생성**: `AWingsPawnBase`를 상속받은 `BP_WingsPawn` 생성.
2. **메쉬 및 물리 설정**: `MeshComponent`에 메쉬 할당 및 `Simulate Physics` 활성화.
3. **카메라 조정**: `SpringArm` 길이(800) 및 오프셋(Z=100) 설정.
4. **월드 배치**: 레벨 배치 후 `Auto Possess Player`를 **Player 0**으로 설정.

### 5. Success Criteria
- 플레이 시 기체가 중력에 의해 자연스럽게 낙하하면 성공.
- 카메라가 기체 후방 상단에서 적절한 구도를 유지하면 성공.
- `LogWings`를 통해 풍향 데이터 로드 로그가 확인되면 성공.

### 6. 검증 방법
- PIE 실행 후 기체의 물리 낙하 및 카메라 트래킹 수동 확인.

---

## [1주차 2일차] 입력 바인딩 및 상태 정의 (완료)

### 1. 목표
- 기체의 3가지 상태(Ready, Flying, Crashed) 정의 및 전환 로직 구현.
- Enhanced Input 시스템을 WingsPawnBase에 연동.
- Ready 상태에서 마우스 움직임(Aim)에 따라 기체가 회전하는 로직 구현.

### 2. 영향 범위
- `ProjectWings/Source/ProjectWings/Public/Pawn/WingsPawnBase.h` (상태 Enum 추가 및 입력 함수 선언)
- `ProjectWings/Source/ProjectWings/Private/Pawn/WingsPawnBase.cpp` (입력 바인딩 및 로직 구현)
- `Content/Input/` (입력 에셋 생성 필요)

### 3. 상세 단계 (C++ Implementation)
1. 상태 Enum 정의: `EWingsPawnState`를 생성하여 상태 관리.
2. 입력 에셋 참조: `UWingsInputConfigData`와 `UInputMappingContext` 포인터를 `WingsPawnBase`에 추가.
3. 입력 바인딩: `SetupPlayerInputComponent`에서 `IA_Aim` 액션을 바인딩.
4. 조준 로직: `Ready` 상태일 때 마우스 입력값에 따라 `MeshComponent`의 `RelativeRotation`을 변경.

### 4. Editor Workflow (중요)
1. 입력 에셋 생성:
   - `Content/Input` 폴더 생성.
   - `Input Action` 생성: `IA_Aim` (Value Type: Axis2D), `IA_Launch` (Digital).
   - `Input Mapping Context` 생성: `IMC_WingsPlayer`.
     - `IA_Aim` 등록 -> `Mouse XY 2D-Axis` 할당.
     - `IA_Launch` 등록 -> `Left Mouse Button` 할당.
2. Data Asset 생성:
   - `UWingsInputConfigData` 기반의 Data Asset(`DA_WingsInput`) 생성 및 액션 할당.
3. Pawn 설정:
   - `BP_WingsPawn`에 `IMC_WingsPlayer`와 `DA_WingsInput` 할당.

### 5. Success Criteria
- 플레이 시작 시 기체가 바닥에 떨어지지 않고 공중에 고정됨 (`Ready` 상태 물리 일시 정지).
- 마우스를 움직일 때 기체가 마우스 방향을 따라 위/아래/좌/우로 회전함.

### 6. 검증 방법
- PIE 실행 후 마우스 조작을 통해 기체의 회전 반응 확인.
- Ready 상태에서 기체가 허공에 잘 떠 있는지 확인.

---

## [1주차 3일차] 포트리스식 발사 시스템 (완료)

### 1. 목표
- 마우스 왼쪽 버튼 홀드 시 파워 충전 로직 구현.
- 버튼 해제 시 충전된 파워에 비례한 발사 추진력 부여.
- 충전 중 시각적 피드백 (로그 및 변수 실시간 업데이트).

### 2. 영향 범위
- `ProjectWings/Source/ProjectWings/Public/Pawn/WingsPawnBase.h` (파워 관련 변수 추가)
- `ProjectWings/Source/ProjectWings/Private/Pawn/WingsPawnBase.cpp` (충전 및 발사 로직 고도화)

### 3. 상세 단계 (C++ Implementation)
1. **변수 추가**: `CurrentLaunchPower` (0.0~1.0), `MaxLaunchForce`, `ChargeSpeed` 선언.
2. **입력 바인딩 수정**: `IA_Launch` 액션을 `Started`(충전 시작)와 `Completed`(발사)로 분리하여 바인딩.
3. **충전 로직 (`Tick`)**: 충전 상태일 때 `DeltaTime`을 이용하여 `CurrentLaunchPower`를 0에서 1까지 점진적으로 증가.
4. **발사 로직 (`Launch`)**: `InitialLaunchForce` 대신 `MaxLaunchForce * CurrentLaunchPower`를 사용하여 최종 힘 계산.

### 4. Editor Workflow (중요)
1. **스탯 설정**:
   - `BP_WingsPawn`에서 `MaxLaunchForce`: **1,000,000.0** (기본값의 2배) 설정.
   - `ChargeSpeed`: **0.5** (2초 동안 풀차지) 설정.
2. **디버깅**:
   - `Tick` 함수에서 충전 중일 때 `GEngine->AddOnScreenDebugMessage`를 사용하여 현재 파워를 화면에 표시.

### 5. Success Criteria
- 왼쪽 마우스 버튼을 누르고 있는 동안 파워가 증가하면 성공.
- 버튼을 짧게 누르면 살짝 날아가고, 길게 누르면 멀리 날아가면 성공.
- 발사 후에는 파워가 0으로 초기화되면 성공.

### 6. 검증 방법
- PIE 실행 후 클릭 길이에 따른 기체의 비행 거리 차이 수동 확인.
- 화면에 출력되는 파워 수치가 0~1 사이에서 정상 동작하는지 확인.

---

## [1주차 4일차] 궤적 가이드라인 구현 (완료)

### 1. 목표
- 기체 발사 전(Ready 상태) 예상 비행 궤적을 시각적으로 표시하여 플레이어의 조준을 돕는다.
- 파워 충전량 및 조준 각도에 따라 실시간으로 변화하는 궤적을 구현한다.

### 2. 영향 범위
- `ProjectWings/Source/ProjectWings/Public/Pawn/WingsPawnBase.h`
- `ProjectWings/Source/ProjectWings/Private/Pawn/WingsPawnBase.cpp`

### 3. 상세 단계 (C++ Implementation)
1. **변수 추가**: `bShowTrajectory`, `TrajectoryMaxTime`, `TrajectoryFrequency`, `TrajectoryRadius` 선언.
2. **궤적 업데이트 로직**: `UpdateTrajectory()` 함수 구현. `UGameplayStatics::PredictProjectilePath`를 사용하여 물리 궤적 계산.
3. **실시간 갱신**: `Tick()` 함수에서 `Ready` 상태일 때 매 프레임 `UpdateTrajectory()` 호출.
4. **시각화**: `FPredictProjectilePathParams`의 `DrawDebugType`을 `ForOneFrame`으로 설정하여 디버그 라인 출력.

### 4. Editor Workflow (중요)
1. **궤적 설정**:
   - `BP_WingsPawn`에서 `TrajectoryMaxTime`: **3.0**, `TrajectoryFrequency`: **15.0**, `TrajectoryRadius`: **10.0** 설정.
2. **조작**:
   - 마우스를 움직여 조준을 변경하거나 왼쪽 버튼을 눌러 파워를 충전할 때 가이드라인이 실시간으로 변하는지 확인.

### 5. Success Criteria
- 조준 각도를 바꿀 때 궤적이 즉각적으로 반응함.
- 파워 충전 중일 때 궤적이 충전량에 비례하여 길어짐.
- 실제 발사 시 기체가 표시된 가이드라인을 거의 정확하게 따라감.

### 6. 검증 방법
- PIE 실행 후 가이드라인이 가리키는 특정 지점을 조준하여 발사.
- 기체가 해당 지점 근처에 착지하거나 충돌하는지 수동 확인.

