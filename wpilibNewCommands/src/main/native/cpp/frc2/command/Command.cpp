// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "frc2/command/Command.h"

#include "frc2/command/CommandHelper.h"
#include "frc2/command/CommandScheduler.h"
#include "frc2/command/ConditionalCommand.h"
#include "frc2/command/InstantCommand.h"
#include "frc2/command/ParallelCommandGroup.h"
#include "frc2/command/ParallelDeadlineGroup.h"
#include "frc2/command/ParallelRaceGroup.h"
#include "frc2/command/PerpetualCommand.h"
#include "frc2/command/ProxyScheduleCommand.h"
#include "frc2/command/RepeatCommand.h"
#include "frc2/command/SequentialCommandGroup.h"
#include "frc2/command/WaitCommand.h"
#include "frc2/command/WaitUntilCommand.h"
#include "frc2/command/WrapperCommand.h"

using namespace frc2;

Command::~Command() {
  CommandScheduler::GetInstance().Cancel(this);
}

Command& Command::operator=(const Command& rhs) {
  m_isGrouped = false;
  return *this;
}

void Command::Initialize() {}
void Command::Execute() {}
void Command::End(bool interrupted) {}

CommandPtr Command::WithTimeout(units::second_t duration) && {
  return CommandPtr(std::move(*this).TransferOwnership()).WithTimeout(duration);
}

CommandPtr Command::Until(std::function<bool()> condition) && {
  return CommandPtr(std::move(*this).TransferOwnership())
      .Until(std::move(condition));
}

CommandPtr Command::IgnoringDisable(bool doesRunWhenDisabled) && {
  return CommandPtr(std::move(*this).TransferOwnership())
      .IgnoringDisable(doesRunWhenDisabled);
}

CommandPtr Command::WithInterruptBehavior(
    InterruptionBehavior interruptBehavior) && {
  return CommandPtr(std::move(*this).TransferOwnership())
      .WithInterruptBehavior(interruptBehavior);
}

CommandPtr Command::WithInterrupt(std::function<bool()> condition) && {
  return CommandPtr(std::move(*this).TransferOwnership())
      .Until(std::move(condition));
}

CommandPtr Command::BeforeStarting(
    std::function<void()> toRun,
    std::initializer_list<Subsystem*> requirements) && {
  return std::move(*this).BeforeStarting(
      std::move(toRun), {requirements.begin(), requirements.end()});
}

CommandPtr Command::BeforeStarting(
    std::function<void()> toRun, std::span<Subsystem* const> requirements) && {
  return CommandPtr(std::move(*this).TransferOwnership())
      .BeforeStarting(std::move(toRun), requirements);
}

CommandPtr Command::AndThen(std::function<void()> toRun,
                            std::initializer_list<Subsystem*> requirements) && {
  return std::move(*this).AndThen(std::move(toRun),
                                  {requirements.begin(), requirements.end()});
}

CommandPtr Command::AndThen(std::function<void()> toRun,
                            std::span<Subsystem* const> requirements) && {
  return CommandPtr(std::move(*this).TransferOwnership())
      .AndThen(std::move(toRun), requirements);
}

PerpetualCommand Command::Perpetually() && {
  WPI_IGNORE_DEPRECATED
  return PerpetualCommand(std::move(*this).TransferOwnership());
  WPI_UNIGNORE_DEPRECATED
}

CommandPtr Command::Repeatedly() && {
  return CommandPtr(std::move(*this).TransferOwnership()).Repeatedly();
}

CommandPtr Command::AsProxy() && {
  return CommandPtr(std::move(*this).TransferOwnership()).AsProxy();
}

CommandPtr Command::Unless(std::function<bool()> condition) && {
  return CommandPtr(std::move(*this).TransferOwnership())
      .Unless(std::move(condition));
}

CommandPtr Command::FinallyDo(std::function<void(bool)> end) && {
  return CommandPtr(std::move(*this).TransferOwnership())
      .FinallyDo(std::move(end));
}

CommandPtr Command::HandleInterrupt(std::function<void(void)> handler) && {
  return CommandPtr(std::move(*this).TransferOwnership())
      .HandleInterrupt(std::move(handler));
}

void Command::Schedule() {
  CommandScheduler::GetInstance().Schedule(this);
}

void Command::Cancel() {
  CommandScheduler::GetInstance().Cancel(this);
}

bool Command::IsScheduled() const {
  return CommandScheduler::GetInstance().IsScheduled(this);
}

bool Command::HasRequirement(Subsystem* requirement) const {
  bool hasRequirement = false;
  for (auto&& subsystem : GetRequirements()) {
    hasRequirement |= requirement == subsystem;
  }
  return hasRequirement;
}

std::string Command::GetName() const {
  return GetTypeName(*this);
}

bool Command::IsGrouped() const {
  return m_isGrouped;
}

void Command::SetGrouped(bool grouped) {
  m_isGrouped = grouped;
}

namespace frc2 {
bool RequirementsDisjoint(Command* first, Command* second) {
  bool disjoint = true;
  auto&& requirements = second->GetRequirements();
  for (auto&& requirement : first->GetRequirements()) {
    disjoint &= requirements.find(requirement) == requirements.end();
  }
  return disjoint;
}
}  // namespace frc2
