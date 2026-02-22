# Push contract changes to T0-contracts branch

## Purpose

Push contract-related changes from the current branch to **T0-contracts** branch. Only includes allowed paths, no code or build artifacts.

## Allowed paths (only these)

- `specs/_contracts/`
- `docs/`
- `cmake/`
- `.cursor/`
- `.specify/`

## Excluded paths

- `build/`
- `include/`
- `source/`
- `tests/`
- `stub/`

## Steps

1. **Record current branch and ensure contract changes are committed**
   - Note current branch name (e.g., `master`) as **source branch**
   - If there are uncommitted contract changes:
     ```bash
     git add specs/_contracts/ docs/ cmake/ .cursor/ .specify/
     git status
     git commit -m "<describe contract/doc/config changes>"
     ```

2. **Switch to T0-contracts and pull latest**
   ```bash
   git fetch origin T0-contracts
   git checkout T0-contracts
   git pull origin T0-contracts
   ```

3. **Checkout contract paths from source branch**
   ```bash
   git checkout <source-branch> -- specs/_contracts docs cmake .cursor .specify
   ```

4. **Commit and push if there are changes**
   ```bash
   git status
   git add specs/_contracts/ docs/ cmake/ .cursor/ .specify/
   git commit -m "contract: sync from <source-branch>"
   git push origin T0-contracts
   ```

5. **Switch back to source branch**
   ```bash
   git checkout <source-branch>
   ```
