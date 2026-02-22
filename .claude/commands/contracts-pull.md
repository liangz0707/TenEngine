# Pull latest contracts from T0-contracts branch

## Purpose

Pull the latest contract changes from **T0-contracts** branch to the current branch.

## Steps

1. **Fetch and checkout T0-contracts**
   ```bash
   git fetch origin T0-contracts
   git checkout T0-contracts
   git pull origin T0-contracts
   ```

2. **Record the latest commit**
   ```bash
   git log -1 --oneline
   ```

3. **Switch back to source branch and merge**
   ```bash
   git checkout <source-branch>
   git checkout T0-contracts -- specs/_contracts docs cmake .cursor .specify
   ```

4. **Commit if there are changes**
   ```bash
   git status
   git add specs/_contracts/ docs/ cmake/ .cursor/ .specify/
   git commit -m "contract: pull latest from T0-contracts"
   ```

## Allowed paths

- `specs/_contracts/`
- `docs/`
- `cmake/`
- `.cursor/`
- `.specify/`
