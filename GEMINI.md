# Project Context & Agent Operating Rules

## 1. Core Operating Principles
- **Three-Phase Workflow:**
  1. **Exploration:** Analyze existing files and dependencies first. Do not edit code during initial exploration.
  2. **Planning:** Provide a clear, step-by-step implementation plan and wait for confirmation if changes affect core logic or >3 files.
  3. **Execution:** Execute tasks incrementally. Verify build integrity and run relevant unit tests after modifying code.
- **Minimal Touch Principle:** Modify only the files strictly necessary for the assigned task. Avoid arbitrary refactoring or formatting changes to untouched files.
- **No Speculative Dependencies:** Do not add third-party libraries or external dependencies without explicit user authorization.
- **Worktree Isolation & PR Workflow:** ALWAYS perform modifications inside a dedicated Git worktree (`git worktree add` / New Worktree Mode). NEVER edit files in the active workspace directory directly. Upon completing and verifying changes, push the branch, open a Pull Request targeting the `dev` branch (`gh pr create --base development`), and explicitly request a user review.

## 2. Project Architecture & Standards
- **Code Style:** Follow language-standard style guidelines strictly, these can be found in the .clang-format file
- **Type Safety & Strictness:** Maintain strict typing across all modules. Avoid untyped interfaces or bypass mechanisms.
- **Use Interfaces:** Use interfaces to define contracts between modules. This allows for easy mocking and testing.
- **Use Modern C++:** Use modern C++ features (C++23, C++26) where appropriate.
- **Error Handling:** Use explicit, structured error handling and logging. Never silently swallow errors or suppress warnings.
- **Documentation:** Include concise docstrings/comments on public interfaces, key algorithms, and non-obvious code paths.

## 3. Testing & Quality Verification
- **Test-Driven Acceptance:** Every new feature or bug fix must include corresponding unit/integration tests.
- **Pre-Commit Verification:** Run local verification commands before declaring a task complete:
  - **Build Command:** `cmake --build build`
  - **Test Command:** `ctest`
  - **Lint / Format:** Ensure linting checks pass cleanly with zero warnings.
- **Failure Recovery:** If a test fails after your changes, immediately analyze logs, form a hypothesis, and propose a fix. If stuck after 2 attempts, notify the user.

## 4. Safety Constraints & Boundaries
- **Restricted Directories:** Do NOT edit files within `.github/workflows/`, `scripts/deploy/`, `credentials/`, or `.env*` files without explicit confirmation.
- **Terminal Execution:**
  - Read-only commands (`git status`, `ls`, `grep`, `cat`) may run freely.
  - Destructive or network-heavy operations (`rm -rf`, `git push`, database migrations) require user review or sandboxed execution.
- **Secret Hygiene:** Never write or echo secret keys, tokens, or credentials into source files or terminal commands.

## 5. Agent Tools & Skills Reference
- **Modular Skills:** For specialized operations (e.g., database schema changes, API integration), check `.agents/skills/` and load relevant `SKILL.md` workflows.
- **Context Tagging:** Prompting with `@path/to/file` is encouraged to keep the active token context narrow and focused.