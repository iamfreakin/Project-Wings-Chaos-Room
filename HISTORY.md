# Project Wings: Development History

## [2026-05-15] 코드 정리 및 구조 단순화 완료
### Removed
- `AWingsPawnBase`에서 연료(Fuel) 시스템 및 관련 Tick 로직 전면 삭제.
- `AWingsGameMode`의 중복 상태 변수(`bIsWin`, `bIsLoss`) 및 불필요한 게터 삭제.

### Changed
- 충격량(Shockwave) 계산 공식을 `(질량 * 속도) * 상성 배율`로 단순화하여 직관성 및 밸런싱 편의성 향상.
- `AWingsLauncher`의 궤적 예측 시 하드코딩된 질량 대신 실제 기체의 데이터 에셋 질량(`GetPawnMass()`)을 사용하도록 개선.
- `AWingsDestructibleActor`의 연쇄 파괴 깊이(`PropagationDepth`)를 데이터 에셋에서 조절 가능하도록 변경.

### Added
- `UWingsFlightData`에 `PawnMass` (기체 질량) 필드 추가.
- `UWingsDestructionData`에 `PropagationDepth` (연쇄 파괴 깊이) 필드 추가.
- `AWingsPawnBase`에 `GetPawnMass()` 게터 추가.

## [2026-05-14] 파편 자동 제거 시스템 구현 및 UE 5.6 빌드 최적화

### Added
- **파편 자동 제거 시스템 (Fractured Piece Removal)**:
  - `UWingsDestructionData`의 `RemovalDuration` 수치를 기반으로 파괴된 파편이 일정 시간 후 자동으로 사라지는 기능 구현.
  - `bScaleOnRemoval` 옵션을 활성화하여 파편이 서서히 작아지며 사라지는 부드러운 연출 적용.

### Fixed
- **UE 5.6 API 호환성 수정**: 
  - `UGeometryCollectionComponent::RemovalSettings` 제거에 따른 빌드 오류 해결.
  - `FGeometryCollectionEdit`을 사용하여 런타임에 에셋(RestCollection) 데이터를 안전하게 수정하도록 개선.
  - 최신 프로퍼티 명칭(`bRemoveOnMaxSleep`, `MaximumSleepTime` 등) 적용.
- **물리 충돌 버그**: 
  - `AWingsDestructibleActor` 생성자에서 `Simulate Physics`를 기본 활성화하여 파괴 전 물체를 통과하는 현상 방지.

### Improved
- **런타임 성능 최적화**: 파편이 월드에 무한히 남지 않도록 하여 물리 연산 및 렌더링 부하 감소.
- **데이터 기반 제어**: 각 오브젝트 타입별로 제거 시간을 데이터 에셋에서 개별 설정 가능하도록 연동.

## [2026-05-14] 에셋 관리 최적화 및 데이터 기반 재질 시스템 구현

### Added
- **데이터 기반 재질 할당 시스템**:
  - `UWingsDestructionData`에 `ExternalMaterial` 및 `InternalMaterial` 프로퍼티 추가.
  - `AWingsDestructibleActor` 초기화 시 데이터 에셋의 재질을 `GeometryCollection` 슬롯 0, 1에 자동 적용하도록 구현.
  - 파괴 전/후의 시각적 무결성 확보.

### Fixed
- **Git 형상 관리 최적화**: `.gitignore`에 대용량 외부 에셋팩(`VRS_LowPolyNature`) 및 관련 바이너리 제외 규칙 추가.
- **렌더링 버그**: 파괴 전 오브젝트에 머터리얼이 적용되지 않던 현상 해결.

### Technical Details
- `GeometryCollectionComponent::SetMaterial`을 통한 런타임 재질 교체 로직 적용.
- `MarkRenderStateDirty`를 호출하여 재질 변경 사항 즉시 반영 보장.

## [2026-05-13] 승리/패배 시스템 및 이벤트 기반 UI 아키텍처 리팩토링

### Added
- **스테이지 목표(Target) 시스템**:
  - `AWingsDestructibleActor`에 `bIsTarget` 속성을 추가하여 클리어 필수 목표 블록 지정 기능 구현.
  - `AWingsGameMode`에서 월드 내 목표 블록 수를 자동 카운트하고 파괴 현황을 실시간 추적.
- **이벤트 기반(Event-Driven) UI 아키텍처**:
  - `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam`을 이용한 `OnGameStateChanged` 델리게이트 구현.
  - `WingsHUD`에서 이벤트를 구독하여 승리/패배 시 결과 위젯(`Victory`/`GameOver`)을 동적으로 생성 및 팝업.
- **UI 액션 기능**:
  - `UWingsUserWidget`에 `RestartLevel`, `QuitGame`, `LoadNextLevel` 함수를 구현하여 결과 화면 버튼 연동 지원.

### Refactored
- **Decoupling**: 위젯이 매 프레임 GameMode의 상태를 묻는 폴링(Polling) 방식에서, 이벤트 발생 시에만 반응하는 브로드캐스트(Broadcast) 방식으로 리팩토링하여 성능 및 구조 개선.
- **UX**: 게임 종료 시 마우스 커서 활성화 및 입력 모드 전환(`SetInputModeUIOnly`)을 통해 결과 화면 조작 편의성 확보.

## [2026-05-13] 기체 전환 로직 개선 및 발사대 카메라 연출 고도화

### Added
- **수동 기체 전환 시스템**:
  - 기체 추락 시 자동으로 복귀하던 타이머(3.0s)를 제거하고 'R' 키 입력을 통한 수동 복귀 로직 구현.
  - `UWingsInputConfigData`에 `IA_Retry` 추가 및 `WingsPlayerController` 바인딩.
- **발사대 카메라 시스템 (Launcher Camera)**:
  - `AWingsLauncher`에 `SpringArm` 및 `CameraComponent`를 C++로 추가하여 발사대 파묻힘 현상 해결.
  - **Dynamic Look**: 발사대 회전에 따라 카메라가 부드럽게 추적하는 동적 시점 구현.
  - **Charge Feedback**: 발사 게이지 충전량에 비례한 카메라 줌인(Zoom-in) 및 절차적(Procedural) 카메라 흔들림(Shake) 효과 추가.

### Improved
- **UI 가시성 로직**: 재시도 안내 문구가 발사대에 있을 때는 보이지 않고, 기체 추락 상태에서만 나타나도록 `GetRetryUIVisibility` 조건 세분화.

## [2026-05-13] 13일차: 스테이지 관리 시스템 및 기체 수 제한 구현

### Added
- **AWingsGameMode**: 스테이지당 허용된 기체 수(TotalSpawnsAllowed) 및 현재 사용량(CurrentSpawnCount) 관리 로직 추가.
- **Retry System**: 기체 추락 후 3초 뒤 발사대로 자동 복귀하여 다시 발사할 수 있는 재시도 루프 구현.
- **UI Integration**: HUD 위젯에서 남은 기체 수를 실시간으로 가져올 수 있는 GetRemainingSpawns() 함수 추가.

### Changed
- **AWingsLauncher**: 기체 발사 성공 시 GameMode에 알림을 보내 카운트가 증가하도록 수정.
- **AWingsPawnBase**: EWingsPawnState::Crashed 상태 진입 시 GameMode에 추락 알림을 보내 재시도 여부를 판단하도록 수정.
- **AWingsPlayerController**: 발사대로 조종권을 돌려받고 입력 컨텍스트를 복구하는 ReturnToLauncher() 기능 추가.

## [2026-05-12] UI 시스템 구현 및 상태 가시성 최적화

### Added
- **UI 프레임워크 기반 마련**: `WingsUserWidget`을 활용한 인게임 HUD 시스템 구현.
- **상태 기반 UI 가시성 제어**: `GetFlightUIVisibility`, `GetLauncherUIVisibility`를 통해 게임 상태에 따른 UI 동적 전환 로직 추가.
- **실시간 데이터 바인딩**:
    - 속도(km/h) 표시 및 연료 잔량(0-1 비율)에 따른 컬러 피드백(Green/Yellow/Red).
- **안전성 로직**: 발사 충돌 시나리오 대비 안전성 확보 로직 추가.

### Improved
- **레이아웃 최적화**: 사용되지 않는 UI 요소에 대한 `Collapsed` 처리를 통해 렌더링 성능 최적화 및 UX 개선.

## [2026-05-12] Chaos 파괴 시스템 고도화 및 UE 5.6 호환성 확보

### Added
- **질량 연동 파괴 시스템**:
  - `UWingsFlightData`에 `DestructionMassReference` 변수 추가 (파괴력 계산 기준 질량).
  - `AWingsPawnBase::SpawnDestructionField` 로직에서 기체의 실제 질량(Mass) 계수 반영.
  - 질량에 비례하여 충격파 강도(`Strength`)와 반경(`Radius`)이 증폭되도록 설계.
- **연쇄 파괴(Chain Destruction) 로직**:
  - `AWingsDestructibleActor`에서 `OnChaosBreakEvent`를 활용한 주변 파괴력 전파(`ApplyExternalStrain`) 구현.
  - 파편 충돌 데미지 활성화 옵션 추가.

### Fixed
- **UE 5.6 C1083 헤더 미포함 오류**:
  - `Chaos/ChaosGameplayEventTypes.h` 경로가 UE 5.6에서 변경됨에 따라 `Chaos/ChaosGameplayEventDispatcher.h`로 인클루드 경로 수정.
  - `ProjectWings.Build.cs`에 `Chaos` 모듈 의존성 추가.
- **파괴력 제한 문제**: `URadialFalloff`의 `MaxRange` 인자가 하드코딩된 값으로 인해 파괴력이 제한되던 결함 수정.

### Improved
- **물리 분석**: `AWingsPawnBase::OnMeshHit` 시 `NormalImpulse` 값을 로그로 출력하여 파괴 위력 튜닝 지원.
- **코드 무결성**: `WingsPawnBase.cpp` 내 중복 정의된 함수 블록을 제거하여 컴파일 안정성 확보.

## [2026-05-11] 연료 및 감속 시스템 구현

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
- Unreal Engine of `UEnhancedInputLocalPlayerSubsystem`을 활용한 런타임 IMC 교체 시스템 구축.
- `Possess`와 IMC 전환을 원자적으로 처리하여 사용자 경험(UX) 끊김 방지.

## [2026-05-10] 프로젝트 초기화 및 핵심 프레임워크 구축

### Added
- **GEMINI.md (Smart Spec)**: AI 리드 개발 어시스턴트를 위한 워크플로우 및 코딩 표준 정의.
- **핵심 C++ 프레임워크**:
    - `AWingsPawnBase`: 비행 및 조종을 위한 베이스 폰 클래스.
    - `AWingsGameMode`, `AWingsGameState`, `AWingsPlayerController`: 핵심 게임 로직 관리 클래스.
    - `UWingsInputConfigData`: Enhanced Input 시스템용 데이터 구조.
    - `LogWings`: 프로젝트 전용 로그 카테고리.
- **문서화**: 게임 기획 및 2주 완성 커리큘럼을 포함한 `README.md` 고도화.
