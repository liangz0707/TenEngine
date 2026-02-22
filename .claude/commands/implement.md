# Execute implementation tasks

## Purpose

Process and execute all tasks defined in tasks.md

## Build Clarification (CMake Tasks)

Before CMake operations, must clarify:
1. **Build root directory**: Current worktree path and CMakeLists.txt location
2. **Dependency method**: All submodules use source-based inclusion
3. **Platform & compiler**: Target platform, C++ standard, build type

## Third-Party Library Tasks

If tasks include third-party integration, follow `docs/third_party-integration-workflow.md` 7 steps:
1. Version selection
2. **Auto-download (required)** - FetchContent/git submodule/script
3. Configure
4. Install
5. Build test
6. Deploy to project
7. Configure implementation

**Never assume third-party exists** - always execute download.

## Steps

1. **Check prerequisites**
   ```powershell
   .specify/scripts/powershell/check-prerequisites.ps1 -Json -RequireTasks -IncludeTasks
   ```

2. **Check checklists status** (if checklists/ exists):
   - Count total/completed/incomplete items
   - If incomplete: ask user to proceed or halt

3. **Load context**:
   - REQUIRED: tasks.md, plan.md
   - IF EXISTS: data-model.md, contracts/, research.md, quickstart.md

4. **Project Setup Verification**:
   - Verify .gitignore, .dockerignore, etc. based on tech stack
   - Add missing patterns

5. **Parse tasks.md structure**:
   - Task phases: Setup, Tests, Core, Integration, Polish
   - Task dependencies: Sequential vs parallel [P]
   - Execution flow

6. **Execute implementation**:
   - Phase-by-phase execution
   - Respect dependencies
   - Follow TDD approach
   - Mark completed tasks as [X]

7. **Completion validation**:
   - Verify all tasks completed
   - Check features match specification
   - Validate tests pass

## Key Rules

- Before cmake/build: **must confirm build root directory**
- All submodules use **source-based inclusion**
- Mark tasks as [X] when completed
