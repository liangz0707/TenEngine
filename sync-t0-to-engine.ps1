# Sync each T0 branch's src, include, tests, cmake, CMakeLists.txt into Engine/TenEngine-NNN-xxx/
# Run from repo root. 001 and 008 already done; run for 002-007, 009-027.
$branches = @(
  'T0-002-object','T0-003-application','T0-004-scene','T0-005-entity','T0-006-input','T0-007-subsystems',
  'T0-009-render-core','T0-010-shader','T0-011-material','T0-012-mesh','T0-013-resource','T0-014-physics',
  'T0-015-animation','T0-016-audio','T0-017-ui-core','T0-018-ui','T0-019-pipeline-core','T0-020-pipeline',
  'T0-021-effects','T0-022-2d','T0-023-terrain','T0-024-editor','T0-025-tools','T0-026-networking','T0-027-xr'
)
foreach ($b in $branches) {
  $engineDir = 'Engine\' + ($b -replace '^T0-','TenEngine-')
  Write-Host "Branch $b -> $engineDir"
  git checkout $b -- src include tests cmake CMakeLists.txt 2>$null
  if (-not (Test-Path src) -and -not (Test-Path include) -and -not (Test-Path CMakeLists.txt)) { Write-Host "  (no content, skip)"; continue }
  New-Item -ItemType Directory -Force -Path $engineDir | Out-Null
  if (Test-Path src) { Move-Item -Path src -Destination "$engineDir\src" -Force }
  if (Test-Path include) { Move-Item -Path include -Destination "$engineDir\include" -Force }
  if (Test-Path tests) { Move-Item -Path tests -Destination "$engineDir\tests" -Force }
  if (Test-Path cmake) { Move-Item -Path cmake -Destination "$engineDir\cmake" -Force }
  if (Test-Path CMakeLists.txt) { Move-Item -Path CMakeLists.txt -Destination "$engineDir\CMakeLists.txt" -Force }
  Write-Host "  ok"
}
