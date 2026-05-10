# Project Wings: Development History

## [2026-05-10] Project Initialization & Core Framework

### Added
- **GEMINI.md (Smart Spec)**: AI Lead Development Assistant를 위한 워크플로우 및 코딩 표준 정의.
- **Core C++ Framework**:
    - `AWingsPawnBase`: 비행 및 조종을 위한 베이스 폰 클래스.
    - `AWingsGameMode`, `AWingsGameState`, `AWingsPlayerController`: 프로젝트 핵심 게임 로직 관리 클래스.
    - `UWingsInputConfigData`: Enhanced Input 시스템을 위한 데이터 구조 정의.
    - `LogWings`: 프로젝트 전용 로그 카테고리 정의.
- **Documentation**: 게임 기획 및 2주 완성 커리큘럼이 포함된 `README.md` 고도화.

### Technical Details
- Unreal Engine 5.6.1 표준 준수.
- IWYU 및 `TObjectPtr` 적용 준비 완료.
- Chaos Physics 및 Enhanced Input 기반 설계 반영.

## [2026-05-10] Input System & State Machine Implementation

### Added
- **Pawn State Machine**: `EWingsPawnState` (Ready, Flying, Crashed) 정의 및 상태 전환 로직 구현.
- **Enhanced Input Integration**:
    - `IMC_WingsPlayer` 및 `IA_Aim`, `IA_Launch` 연동.
    - `SetupPlayerInputComponent`를 통한 액션 바인딩.
- **Aiming Mechanics**: `Ready` 상태에서 마우스 입력에 따른 기체 Pitch/Yaw 회전 로직 구현.
- **Launch Mechanics**: 발사 시 물리 시뮬레이션 활성화 및 전방 추진력(`AddImpulse`) 부여 기능.

### Fixed
- **Build Error**: `ProjectWings.Build.cs`에 `Niagara` 모듈 누락 문제 해결.
- **Include Error**: `WingsPawnBase.cpp`에서 불필요한 `ProjectWings.h` 참조 제거 및 로그 카테고리 선언 방식 수정.

### Documentation
- `PLANS_DAILY.md`, `PLAN_PROGRESS.md`를 생성하여 체계적인 일정 관리 시작.
- 모든 계획 및 결과 기록에 상세 가이드 형식 적용.
