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

## [1주차 5일차] 3축 비행 물리 및 조종성 리파인 (완료)

### 1. 목표
- 마우스와 키보드를 조합한 하이브리드 비행 조종 시스템 구축.
- 비행 중 회전 감도를 기존 대비 10배 하향하여 묵직하고 정밀한 조종감 제공.
- A/D 입력 시 기체 기울임(Roll)과 동시에 해당 방향으로 옆으로 미는 힘(Lateral Force)을 가해 기동성 개선.
- 뱅크-투-턴(Bank-to-Turn) 및 자동 수평 유지 로직을 추가하여 자연스러운 비행 구현.
- 엔진 기본 중력을 유지하여 환경 파괴 등 물리 상호작용 호환성 확보.

### 2. 영향 범위
- `ProjectWings/Source/ProjectWings/Public/Pawn/WingsPawnBase.h`
- `ProjectWings/Source/ProjectWings/Private/Pawn/WingsPawnBase.cpp`

### 3. 상세 단계 (C++ Implementation)
1. **변수 선언**: 각 축별 감도(`FlightPitchSensitivity` 등), 추진력, 옆으로 미는 힘, 뱅크-투-턴 강도 변수 추가.
2. **입력 바인딩**: `SetupPlayerInputComponent`에서 `IA_Pitch`, `IA_Roll`, `IA_Thrust` 액션 연동.
3. **토크 및 힘 제어**:
   - `Input_Roll`: Roll 토크와 동시에 `AddForce`로 측면 이동 힘 적용.
   - `Input_FlightMouse`: 마우스 입력에 낮은 감도 적용하여 토크 부여.
4. **비행 물리 보정 (`Tick`)**:
   - **속도 정렬**: 진행 방향이 기체의 전방을 서서히 따라가도록 보정 (미끄러짐 방지).
   - **자동 수평 유지**: 입력이 없을 때 기체를 수평으로 서서히 복원.
   - **뱅크-투-턴**: 기체가 기울어진 방향으로 자동으로 선회(Yaw) 토크 부여.

### 4. Editor Workflow (중요)
1. **입력 에셋 설정**:
   - `IA_Pitch`, `IA_Roll`, `IA_Thrust` (Value Type: Axis1D) 생성 및 `IMC_WingsPlayer` 등록.
2. **데이터 에셋 업데이트**:
   - `DA_WingsInput`에 생성한 액션들을 할당.
3. **수치 밸런싱**:
   - `BP_WingsPawn`에서 감도(`1.0`, `0.8`, `1.2`), 옆으로 미는 힘(`15,000`), 뱅크-투-턴 강도(`0.5`) 등 기본값 확인 및 조정.

### 5. Success Criteria
- 마우스 조작 시 회전이 묵직하게 이루어지며 정밀한 조종이 가능함.
- A/D 키를 누를 때 기체가 옆으로 미끄러지듯 이동함.
- 기체를 옆으로 기울이면 자동으로 해당 방향으로 서서히 선회함.
- 조작을 멈추면 기체가 서서히 수평을 되찾음.

### 6. 검증 방법
- PIE 실행 후 기체 발사, 공중에서 3축 조작 및 기동성 확인.
- 궤적 가이드라인이 정상적으로 표시되는지 확인.

---

## [1주차 6일차] 아키텍처 리팩토링: Launcher와 Projectile 분리 (완료)

### 1. 목표
- `WingsPawnBase`에 집중된 책임을 `WingsLauncher`(발사대)와 `WingsPawn`(발사체)으로 분리.
- 발사 전(조준, 충전)과 발사 후(비행)의 상태 머신을 클래스 단위로 분리하여 유지보수성 향상.
- `PlayerController`를 통한 동적 조종권(Possess) 전환 및 입력 컨텍스트(IMC) 교환 시스템 구축.
- 마우스 조작 편의성 개선 (클릭 없이 조준 및 비행 가능).

### 2. 영향 범위
- `ProjectWings/Source/ProjectWings/Public/Pawn/WingsPawnBase.h/cpp`
- `ProjectWings/Source/ProjectWings/Public/Launcher/WingsLauncher.h/cpp`
- `ProjectWings/Source/ProjectWings/Public/Core/WingsPlayerController.h/cpp`

### 3. 상세 단계 (C++ Implementation)
1. **AWingsLauncher 구현**: 조준, 충전, 궤적 예측 로직 이식 및 `ProjectileClass` 스폰 기능 구현.
2. **AWingsPawnBase 경량화**: 발사 관련 레거시 코드 제거 및 비행 물리 전담화.
3. **Controller 로직 업데이트**: 
   - `TransitionToFlight` 구현: 조종권 변경 및 IMC 교체(Launcher IMC 제거 -> Pawn IMC 추가).
   - `BeginPlay`: `FInputModeGameOnly` 설정 및 마우스 커서 숨김으로 조작감 개선.

### 4. Editor Workflow (중요)
1. **블루프린트 설정**: `BP_WingsLauncher` 생성 및 `ProjectileClass`에 `BP_WingsPawn` 할당.
2. **컴포넌트 조정**: `LaunchDirectionIndicator`의 위치를 메쉬 외부로 이동시켜 스폰 충돌 방지.
3. **게임모드 설정**: `World Settings`에서 `Player Controller Class`를 `AWingsPlayerController`로 지정.

### 5. Success Criteria
- 게임 시작 시 발사대 조준 및 파워 충전이 정상 동작함.
- 발사 시 조종권과 카메라가 기체로 부드럽게 전환되며 즉시 비행 조종이 가능함.
- 마우스를 클릭하지 않아도 조준과 비행 방향 전환이 즉각적으로 이루어짐.

### 6. 검증 방법
- PIE 실행 후 발사 시퀀스 전체 테스트 및 마우스 조작감 수동 확인.

---

## [1주차 7일차] 연료 및 감속 시스템 (완료)

### 1. 목표
- 비행 중 지속적으로 소모되는 연료 시스템 구현.
- 연료 소진 시 물리적 감속 및 낙하 로직 추가.
- UI 연동을 위한 기초 데이터 바인딩.

### 2. 영향 범위
- `ProjectWings/Source/ProjectWings/Public/Pawn/WingsPawnBase.h/cpp`

### 3. 상세 단계 (C++ Implementation)
1. **변수 선언**: `MaxFuel`, `CurrentFuel`, `FuelConsumptionRate`, `ThrustFuelMultiplier` 추가.
2. **소모 로직 (`Tick`)**: 비행 중 매 프레임 연료 차감. 추진력 가중치 반영.
3. **고갈 상태 구현**: 연료 0 시 추진력 초기화 및 입력 차단, 선형 감쇠 증가로 추락 유도.
4. **인터페이스 제공**: `GetFuelPercentage()` 등 게터 함수 구현.

### 4. Editor Workflow (중요)
1. **스탯 조정**: `BP_WingsPawn`에서 연료 최대량 및 소모 속도 밸런싱.
2. **피드백 확인**: 화면 디버그 메시지로 연료 소모량 실시간 모니터링.

### 5. Success Criteria
- 비행 중 연료가 정해진 속도에 따라 줄어듬.
- 추진력을 쓰면 소모 속도가 체감될 정도로 빨라짐.
- 연료가 다 떨어지면 조종이 불가능해지며 땅으로 떨어짐.

### 6. 검증 방법
- PIE 실행 후 연료 고갈 시점까지 비행 테스트.

---

## [1주차 8일차] 다이내믹 카메라 및 자유 시점 구현 (예정)

### 1. 목표
- **유연한 추적 카메라**: 기체의 급격한 회전 시 카메라가 부드럽게 지연(Lag)되며 따라오도록 설정하여 물리적 긴장감 부여.
- **동적 시야각(FOV) 및 거리**: 기체의 속도가 빨라지면 FOV가 넓어지고 카메라 거리가 뒤로 멀어지는 효과를 통해 속도감 극대화.
- **자유 시점(Free Look)**: 마우스 우클릭을 유지하는 동안 기체의 방향과 상관없이 주변을 둘러볼 수 있는 기능 구현.
- **언리얼 5.5+ 최신 방식 적용**: `Camera Shake`가 아닌 `Camera Lens Effect` 또는 강화된 `SpringArm` 기능을 활용한 몰입감 개선.

### 2. 영향 범위
- `ProjectWings/Source/ProjectWings/Public/Pawn/WingsPawnBase.h/cpp`
- `ProjectWings/Source/ProjectWings/Public/Data/WingsInputConfigData.h`
- `Content/Input/` (IA_FreeLook 신규 생성 및 IMC 바인딩)

### 3. 상세 단계 (C++ Implementation)
1. **SpringArm 설정 최적화**: `bEnableCameraLag`, `bEnableCameraRotationLag` 활성화 및 속도 수치 변수화.
2. **동적 효과 (Dynamic FOV/Distance)**: `Tick`에서 전방 속도(`DotProduct`) 기반으로 FOV와 `TargetArmLength`를 `FMath::FInterpTo`로 보간 업데이트.
3. **자유 시점 (Free Look) 구현**: `IA_FreeLook` 바인딩, 우클릭 시 `bIsFreeLooking` 활성화 및 카메라 회전 제어 분리. 우클릭 해제 시 부드러운 카메라 복귀 로직 추가.

### 4. Editor Workflow (중요)
1. **입력 에셋 설정**: `IA_FreeLook` (Digital) 생성 및 `IMC_WingsPlayer`에 `Right Mouse Button` 등록.
2. **데이터 에셋 업데이트**: `DA_WingsInput`에 생성한 액션 할당.
3. **수치 밸런싱**: `BP_WingsPawn`에서 카메라 지연 속도, FOV 최소/최대값, ArmLength 가중치 조정.

### 5. Success Criteria
- 기체 회전 시 카메라가 부드럽게 뒤따라오며(Lag), 속도가 빨라질수록 시야가 넓어짐(FOV).
- 우클릭을 누른 상태에서 마우스를 움직여 기체 주변을 360도 자유롭게 관찰 가능.
- 우클릭을 떼면 카메라가 기체 정후방으로 자연스럽게 복귀.

### 6. 검증 방법
- PIE 실행 후 가속/감속 및 급선회 시 카메라 연출 확인.
- 우클릭 조작을 통한 자유 시점 및 복귀 트리거 테스트.

