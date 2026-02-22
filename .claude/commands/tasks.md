# Generate task breakdown

## Purpose

Break down implementation plan into executable tasks.

## Steps

1. **Load plan context**
   - Read plan.md for architecture and tech stack
   - Read data-model.md for entities
   - Read contracts/ for API/ABI requirements

2. **Generate tasks by phase**:

   ### Setup Phase
   - Project structure initialization
   - Dependency configuration
   - Build system setup

   ### Test Phase (TDD)
   - Unit test scaffolding
   - Integration test setup
   - Contract compliance tests

   ### Core Phase
   - Model/entity implementation
   - Service logic implementation
   - API/ABI implementation

   ### Integration Phase
   - Upstream module integration
   - Third-party library integration
   - Cross-module coordination

   ### Polish Phase
   - Performance optimization
   - Documentation
   - Final validation

3. **Format tasks.md**:
   ```markdown
   ## Phase: Setup
   - [ ] T001: Task description (file: path/to/file.cpp)
   - [ ] T002: Task description [P] (parallel)

   ## Phase: Core
   - [ ] T003: Depends on T001
   ```

4. **Add test requirements**:
   - Tests must cover upstream module capabilities
   - Tests must cover third-party library calls
   - Not just isolated module logic

## Task Markers

- `[P]` - Can run in parallel
- No marker - Must run sequentially
- Dependency noted in description
