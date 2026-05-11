# Project Wings: Development History

## [2026-05-10] 프로젝트 초기화 및 핵심 프레임워크 구축

### Added
- **GEMINI.md (Smart Spec)**: AI 리드 개발 어시스턴트를 위한 워크플로우 및 코딩 표준 정의.
- **핵심 C++ 프레임워크**:
    - `AWingsPawnBase`: 비행 및 조종을 위한 베이스 폰 클래스.
    - `AWingsGameMode`, `AWingsGameState`, `AWingsPlayerController`: 핵심 게임 로직 관리 클래스.
    - `UWingsInputConfigData`: Enhanced Input 시스템용 데이터 구조.
    - `LogWings`: 프로젝트 전용 로그 카테고리.
- **문서화**: 게임 기획 및 2주 완성 커리큘럼을 포함한 `README.md` 고도화.

### Technical Details
- Unreal Engine 5.6.1 표준 준수.
- IWYU 및 `TObjectPtr` 적용.
- Chaos Physics 및 Enhanced Input 기반 설계 반영.

## [2026-05-10] 입력 시스템 및 상태 머신 구현

### Added
- **폰 상태 머신**: `EWingsPawnState` (Ready, Flying, Crashed) 정의 및 상태 전환 로직 구현.
- **Enhanced Input 통합**:
    - `IMC_WingsPlayer` 및 `IA_Aim`, `IA_Launch` 연동.
    - `SetupPlayerInputComponent`를 통한 액션 바인딩.
- **조준 메커니즘**: `Ready` 상태에서 마우스 입력에 따른 기체의 Pitch/Yaw 회전 로직 구현.
- **발사 메커니즘**: 발사 시 물리 시뮬레이션 활성화 및 전방 추진력(`AddImpulse`) 부여 기능.

### Fixed
- **빌드 오류**: `ProjectWings.Build.cs`에 `Niagara` 모듈 누락 문제 해결.
- **인클루드 오류**: `WingsPawnBase.cpp`에서 불필요한 `ProjectWings.h` 참조 제거 및 로그 카테고리 선언 방식 수정.

### Documentation
- `PLANS_DAILY.md`, `PLAN_PROGRESS.md`를 생성하여 체계적인 일정 관리 시작.
- 모든 계획 및 결과 기록에 상세 가이드 형식 적용.

## [2026-05-10] 발사 시스템 및 문서화 표준 확립

### Added
- **포트리스식 발사 시스템**:
    - 입력 이벤트(`Started`, `Completed`)를 이용한 파워 충전 로직 구현.
    - 충전된 파워(0.0 ~ 1.0)에 따른 가변 임펄스 적용.
    - 화면 디버그 메시지를 통한 실시간 파워 충전 피드백 제공.
- **문서화 표준 (GEMINI.md)**:
    - 일관된 6단계 계획 및 히스토리 추적을 위한 "12. 문서화 표준" 명문화.
    - 완료된 작업의 상세 내용을 `PLANS_DAILY.md`에 보존하도록 강제.

### Improved
- **폰 물리**: 발사 후 비행 안정성을 위해 선형/각성 댐핑 수치 조정.
- **UI/UX**: 충전 중 파워 레벨을 시각화하기 위해 `GEngine` 디버그 메시지 추가.

## [2026-05-11] 3축 비행 물리 및 조종성 리파인

### Added
- **3축 비행 제어 시스템**: 마우스 및 키보드 입력을 통한 Pitch, Yaw, Roll 물리 기반 제어 구현.
- **비행 물리 보정 로직**:
  - **Velocity Alignment**: 진행 방향이 기체 정면을 따라가도록 보정하여 미끄러짐 현상 완화.
  - **Auto-Leveling**: 입력 부재 시 기체를 수평으로 서서히 복원하는 기능 추가.
  - **Bank-to-Turn**: 기체 기울임에 따라 자동으로 선회(Yaw) 토크를 부여하는 선회 보정 시스템.
- **추진력 시스템**: 비행 중 추진력을 조절(W/S)하고 지속적인 힘을 가하는 로직 구현.
- **조종성 향상**: Roll 입력 시 측면 이동 힘(Lateral Force)을 부여하여 기민한 기동 가능.

### Changed
- `AWingsPawnBase` 생성자에서 비행 물리 관련 변수들(감도, 힘, 속도 등)의 기본값 초기화.
- `SetupPlayerInputComponent`에서 비행용 입력 액션(`IA_Pitch`, `IA_Roll`, `IA_Thrust`) 바인딩 추가.
- `Tick` 함수 내에 `Flying` 상태 전용 물리 시뮬레이션 로직 통합.

## [2026-05-11] 궤적 가이드라인 구현

### Added
- **예상 궤적 시각화**:
    - `UGameplayStatics::PredictProjectilePath`를 이용한 물리 기반 비행 경로 예측 로직 구현.
    - `Ready` 상태에서 마우스 조준 및 파워 충전에 따른 실시간 궤적 갱신 기능 추가.
    - 엔진 디버그 드로잉을 통한 즉각적인 궤적 가이드라인 표시.
- **궤적 커스텀 변수**:
    - `TrajectoryMaxTime`, `TrajectoryFrequency`, `TrajectoryRadius` 등을 통해 궤적의 정밀도와 길이를 에디터에서 조절 가능하도록 구현.

### Technical Details
- `FPredictProjectilePathParams`를 활용하여 실제 물리 엔진과 동일한 중력 및 충돌 판정을 궤적에 반영.
- `Tick` 내에서의 효율적인 업데이트를 위해 조준 상태(`Ready`)에서만 연산 수행.

## [2026-05-11] 아키텍처 리팩토링 결정 및 설계

### Added
- **Launcher-Projectile 분리 설계**:
    - `AWingsPawnBase`에 집중된 조준/충전/비행 로직을 `AWingsLauncher`와 `AWingsPawn`으로 분리하기 위한 설계 확정.
    - `AWingsPlayerController`를 통한 동적 조종권 전환 시나리오 수립.

### Planned
- `AWingsLauncher` 클래스 신규 생성 및 조준/궤적 로직 이식.
- `AWingsPawnBase`에서 발사 관련 레거시 코드 제거 및 비행 전담화.

## [2026-05-11] WingsLauncher 컴파일 오류 해결

### Fixed
- **`WingsLauncher.h` 식별자 오류**: `AWingsPawnBase` 클래스에 대한 전방 선언(Forward Declaration) 누락으로 인한 `TSubclassOf` 템플릿 인수 오류 해결.
- **`WingsLauncher.cpp` 참조 오류**: `WingsPawnBase.h` 및 `WingsPlayerController.h` 헤더 포함 누락으로 인한 `SpawnActor` 및 `Cast` 호출 오류 해결.
- **빌드 타겟 수정**: 빌드 스크립트 실행 시 `ProjectWingsEditor` 타겟을 명시하여 빌드 환경 정규화.

### Technical Details
- Unreal Engine의 IWYU(Include What You Use) 원칙에 따라 필요한 최소한의 헤더만 포함하여 컴파일 속도 최적화.
- `TSubclassOf` 사용 시 불완전한 형식(Incomplete Type) 문제를 전방 선언으로 해결.

## [2026-05-11] 아키텍처 리팩토링: Launcher-Projectile 분리 및 입력 전환 구현

### Added
- **`AWingsLauncher` 클래스 활성화**:
  - `BeginPlay`에서 `LauncherMappingContext`를 동적으로 추가하는 로직 구현.
  - `GetLauncherMappingContext()` 게터를 통해 컨트롤러와의 상호작용 지원.
- **`AWingsPlayerController` 기능 확장**:
  - `TransitionToFlight(APawn* FlightPawn, APawn* PreviousPawn)` 구현.
  - 조종권 전환 시 이전 IMC 제거 및 새로운 IMC 주입 로직 통합.
- **`AWingsPawnBase` 게터 추가**:
  - `GetDefaultMappingContext()`를 통해 컨트롤러에서 IMC를 제어할 수 있도록 개선.

### Changed
- **입력 흐름 개선**: 기체(`WingsPawnBase`)의 `BeginPlay`에서 IMC를 추가하던 방식을 제거하고, 컨트롤러가 전환 시점에 명시적으로 관리하도록 변경하여 입력 충돌 방지.
- **클래스 캡슐화**: 필요한 프로퍼티와 게터를 정돈하여 클래스 간 의존성을 명확히 정의.

### Technical Details
- Unreal Engine의 `UEnhancedInputLocalPlayerSubsystem`을 활용한 런타임 IMC 교체 시스템 구축.
- `Possess`와 IMC 전환을 원자적으로 처리하여 사용자 경험(UX) 끊김 방지.

## [2026-05-11] 연료 및 감속 시스템 구현 (Day 7)

### Added
- **연료 시스템 구축**:
  - `AWingsPawnBase`에 연료 잔량, 최대 연료량, 소모율 변수 추가.
  - 비행 상태에서 추진력 사용량에 비례한 실시간 연료 차감 로직 구현.
- **연료 고갈 패널티**:
  - 연료가 바닥났을 때 추진력 입력을 차단하고 기존 추진력을 0으로 초기화.
  - 선형 감쇠(Linear Damping)를 대폭 높여 기체가 힘없이 떨어지는 물리 효과 적용.
- **데이터 게터**: `GetFuelPercentage()` 등을 통해 UI 연동이 용이하도록 인터페이스 제공.

### Improved
- **디버그 피드백**: 실시간 연료 상태를 화면 디버그 메시지로 출력하여 즉각적인 상태 확인 지원.
- **입력 무결성**: 연료가 없는 상태에서는 엔진 가속 입력이 무시되도록 예외 처리 추가.

### Technical Details
- `FMath::Max` 및 `FMath::Clamp`를 사용하여 연료 수치가 음수가 되지 않도록 보호.
- 물리 엔진의 Damping 계수를 동적으로 조절하여 엔진 정지 상황을 시각화.

