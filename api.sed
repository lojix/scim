/^int/ {
 s/^/extern /g
 s/$/;\n/g
 p
}
