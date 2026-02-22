# Write back ABI/API changes to contract files

## Purpose

After plan phase generates ABI/API modifications, write them back to the contract files:
- `specs/_contracts/NNN-modulename-ABI.md`
- `specs/_contracts/NNN-modulename-public-api.md`

## When to use

After running `/plan` command, if the plan includes ABI/API changes.

## Steps

1. **Review plan output**
   - Check `contracts/` directory in feature folder for generated ABI changes
   - Review new/modified entries

2. **Update ABI file**
   - For new entries: append to appropriate section in ABI file
   - For modified entries: update existing entries
   - For removed entries: mark as deprecated or remove

3. **Update public-api if needed**
   - If new capabilities are added, update public-api
   - Maintain API as source of truth for ABI

4. **Clear TODOs**
   - If implementing an ABI TODO, remove the TODO marker after implementation

5. **Commit changes**
   ```bash
   git add specs/_contracts/
   git commit -m "contract: update NNN-modulename ABI/API"
   ```

6. **Push to T0-contracts**
   - Run `/contracts-push` to sync to T0-contracts branch

## Notes

- After writeback, user may run `/contracts-downstream-todo` to add TODO notes to downstream modules
