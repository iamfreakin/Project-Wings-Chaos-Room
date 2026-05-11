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
