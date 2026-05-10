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
