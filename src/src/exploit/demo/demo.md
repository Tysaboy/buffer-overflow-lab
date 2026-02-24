# Demo --- Buffer Overflow Exploit & Fix Verification

This demo shows:

1.  The vulnerable server can be exploited.
2.  The patched server resists the same exploit.

------------------------------------------------------------------------

## Step 1 --- Compile Vulnerable Server

``` bash
gcc -O0 -fno-stack-protector -z execstack -no-pie -o vuln_server src/vuln_server.c
```

Run it:

``` bash
./vuln_server 22222
```

You should see:

    [vuln] listening on 127.0.0.1:22222

------------------------------------------------------------------------

## Step 2 --- Run Exploit (In Another Terminal)

``` bash
python3 exploit/exploit.py 22222
```

Expected result:

    [+] BUFSZ guess = <some number>
    The secret is: FLAG{demo_secret_do_not_use_in_prod}

This confirms the buffer overflow allows authentication bypass.

------------------------------------------------------------------------

## Step 3 --- Compile Patched Server

Stop the vulnerable server.

Then compile:

``` bash
gcc -O2 -o fixed_server src/fixed_server.c
```

Run:

``` bash
./fixed_server 22222
```

You should see:

    [fixed] listening on 127.0.0.1:22222

------------------------------------------------------------------------

## Step 4 --- Run Exploit Again

``` bash
python3 exploit/exploit.py 22222
```

Expected result:

    [-] exploit failed (target likely patched or different layout)

The server responds with:

    Wrong password.

This confirms the vulnerability has been eliminated by proper bounds
checking.

------------------------------------------------------------------------

## Conclusion

-   The vulnerable server allows memory overwrite of adjacent data.
-   The exploit automates discovery of buffer size.
-   The fixed implementation enforces correct bounds validation.
-   The same exploit fails against the patched server.

Exploit → Verify → Patch → Confirm.
