# Add TODO compatibility notes to downstream modules

## Purpose

When a module's API changes, add TODO notes to downstream modules' public-api files so they can adapt to the changes.

## When to use

After `/contracts-writeback` if the contract includes a "## Change Log" section.

## Steps

1. **Identify changed module**
   - Check which module's contract was updated
   - Review change log for breaking/non-breaking changes

2. **Find dependent modules**
   - Check `specs/_contracts/000-module-dependency-map.md`
   - List all modules that depend on the changed module

3. **Add TODO notes to downstream public-api files**
   - For each dependent module, add to their public-api:
   ```markdown
   <!-- TODO: Adapt to upstream <module> changes (YYYY-MM-DD) -->
   <!-- - <specific change description> -->
   ```

4. **Commit changes**
   ```bash
   git add specs/_contracts/
   git commit -m "contract: add downstream TODO for <module> changes"
   ```

## Note

This command is **not automatic** - user must explicitly invoke it.
