# Project-Wings-Chaos-Room

발사할 기체를 선택하고, 실시간 3축 비행으로 목표 지점에 충돌시켜 속성이 일치하는 구조물만 Chaos Physics로 파괴하는 UE5.6 물리 퍼즐 프로토타입입니다.

[Repository](https://github.com/iamfreakin/Project-Wings-Chaos-Room) | [Portfolio Hub](https://github.com/iamfreakin/GamePortfolio) | [GDD](GDD.md)

## Overview

| 항목 | 내용 |
| --- | --- |
| 장르 | 물리 기반 항공 파괴 퍼즐 액션 |
| 엔진/언어 | Unreal Engine 5.6, C++ |
| 개발 상태 | Phase 4 완료 후 개발 중단, NEONDRIFT로 전환 |
| Repository | [iamfreakin/Project-Wings-Chaos-Room](https://github.com/iamfreakin/Project-Wings-Chaos-Room) |

## Problem

발사체를 쏘고 끝나는 구조가 아니라, 발사 후 플레이어가 직접 비행을 보정하면서 속성이 맞는 목표 구조물만 파괴해야 했습니다. 단순 물리 충돌과 퍼즐 규칙이 섞이면 속성 불일치 상황에서도 구조물이 깨질 수 있으므로, 파괴 조건을 코드에서 명확히 제어해야 했습니다.

## Core Loop

1. 기체 선택 UI에서 속성 기체를 시퀀스로 배치합니다.
2. 발사대에서 조준과 파워를 충전해 발사합니다.
3. 발사 후 플레이어가 pitch, yaw, roll, thrust로 기체를 조종합니다.
4. 기체와 타겟의 속성이 일치할 때만 Chaos 파괴가 발생합니다.
5. 확정한 시퀀스 순서대로 기체를 소진하고 모든 목표물을 파괴하면 클리어합니다.

## Implementation

`AWingsLauncher`는 기체 선택 시퀀스가 확정된 뒤에만 충전 발사를 허용합니다. 발사 완료 시 GameMode에서 다음 기체 클래스를 받아 스폰하고, 초기 impulse를 준 뒤 PlayerController의 조종권을 발사대에서 비행 Pawn으로 전환합니다.

`AWingsPawnBase::OnMeshHit`에서는 기체 속성과 타겟 속성을 비교합니다. 속성이 일치하거나 `Universal`이면 Geometry Collection을 Dynamic으로 전환하고 Field System으로 파괴 field를 생성합니다. 속성이 맞지 않으면 damage threshold를 높여 물리 충돌은 허용하되 파괴를 차단합니다.

## My Contribution

- `AWingsLauncher` 기반 조준, 충전 발사, 궤적 예측, 기체 스폰
- `AWingsPawnBase` 기반 pitch/yaw/roll/thrust 3축 비행
- velocity alignment, fake lift, bank-to-turn 기반 비행 보정
- `EWingsAttribute` 기반 기체/타겟 속성 판정
- `AWingsDestructibleActor` 기반 Geometry Collection 상태 제어
- Field System 기반 Chaos 파괴 충격파 생성
- `AWingsGameMode` 기반 기체 시퀀스, 타겟 카운트, 승패 판정
- 마지막 기체와 마지막 타겟 파괴가 겹치는 레이스 컨디션 완화

## Tech Stack

| 영역 | 내용 |
| --- | --- |
| Engine | Unreal Engine 5.6 |
| Physics | Chaos Physics, Geometry Collection, Field System |
| Input | Enhanced Input |
| Architecture | Data Asset 기반 비행/파괴 수치 관리 |

## Current Scope

이 프로젝트는 Phase 4 완료 후 NEONDRIFT로 전환되며 개발이 중단되었습니다. 정식 스테이지 디자인, 사운드, 메인 메뉴, 기체 외형 에셋은 미완성 범위로 분리합니다.
