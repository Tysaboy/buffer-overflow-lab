# Buffer Overflow Lab --- Exploit → Automate → Patch (C)

This repository is a clean-room educational case study demonstrating how
a **stack-based buffer overflow** can be used to bypass authentication
--- and how proper bounds checking eliminates the vulnerability.

> ⚠️ For educational purposes only. The vulnerable server binds to
> `127.0.0.1` and must never be deployed.

------------------------------------------------------------------------

## Overview

This project demonstrates the full security lifecycle:

-   Identify a memory safety vulnerability
-   Exploit it to bypass authentication
-   Automate the exploit
-   Implement a secure fix
-   Verify the exploit fails after patching

Exploit → Verify → Patch → Confirm.

------------------------------------------------------------------------

## What This Demonstrates

-   Unsafe input handling in C
-   Stack memory layout implications
-   Overwriting adjacent memory (authentication bypass)
-   Null byte (`\0`) string termination behavior
-   Automated exploitation across unknown buffer sizes
-   Defensive programming with correct bounds enforcement

------------------------------------------------------------------------

## Repository Structure

    buffer-overflow-lab/
    ├── src/
    │   ├── vuln_server.c
    │   └── fixed_server.c
    ├── exploit/
    │   └── exploit.py
    ├── demo/
    │   └── demo.md
    └── docs/

------------------------------------------------------------------------

## Threat Model

An attacker can connect to the server and send arbitrarily long input.

Goal: Reveal a protected secret without knowing the legitimate password.

------------------------------------------------------------------------

## Root Cause Analysis

The vulnerable implementation reads bytes into a fixed-size buffer but
uses an incorrect stopping condition.

Instead of enforcing:

    buffer + BUFSZ - 1

it stops only when reaching an address near `secret`.

This allows writing beyond the buffer boundary and into adjacent memory
--- specifically the stored password.

This is a classic memory safety failure caused by improper bounds
validation.

------------------------------------------------------------------------

## Exploit Strategy (High-Level)

1.  Send a chosen password (e.g., `hack\0`) at the start of the input.
2.  Pad input until overwrite reaches the stored password.
3.  Overwrite the stored password with the same chosen value.
4.  `strcmp(input, stored_password)` succeeds.
5.  Secret is revealed.

The exploit script automates this process and brute-forces the correct
buffer size within a safe range.

------------------------------------------------------------------------

## Build & Run

### Compile Vulnerable Server

``` bash
gcc -O0 -fno-stack-protector -z execstack -no-pie -o vuln_server src/vuln_server.c
```

Run:

``` bash
./vuln_server 22222
```

------------------------------------------------------------------------

### Run Exploit

In another terminal:

``` bash
python3 exploit/exploit.py 22222
```

Expected result:

    [+] BUFSZ guess = <value>
    The secret is: FLAG{demo_secret_do_not_use_in_prod}

------------------------------------------------------------------------

### Compile Patched Server

``` bash
gcc -O2 -o fixed_server src/fixed_server.c
```

Run:

``` bash
./fixed_server 22222
```

------------------------------------------------------------------------

### Run Exploit Again

``` bash
python3 exploit/exploit.py 22222
```

Expected result:

    [-] exploit failed (target likely patched or different layout)

Server response:

    Wrong password.

------------------------------------------------------------------------

## Security Lessons

-   A single incorrect bounds check can break memory safety.
-   Exploitation often depends on predictable layout and string
    semantics.
-   Defensive coding must enforce strict buffer limits.
-   Verification after patching is essential.

------------------------------------------------------------------------

## Author

Gildas Yegnon\
Computer Science --- University of Calgary\
CompTIA Security+\
Aspiring Offensive Security Engineer
