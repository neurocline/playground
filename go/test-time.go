// test-time.go

package main

import (
	"fmt"
	"os"
	"os/user"
	"time"
)

func main() {
	realdate := time.Now().Format("Mon Jan _2 15:04:05 2006")
	fmt.Printf("Now is %s\n", realdate)

	var username, hostname string

	myself, err := user.Current()
	if err == nil {
		fmt.Printf("Whoami: %+v\n", myself)
		username = myself.Username
		fmt.Printf("I am %s\n", myself.Username)
	} else {
		fmt.Printf("user.Lookup failed: %s", err)
	}

	hostname, err = os.Hostname()
	if err == nil {
		fmt.Printf("Hostname: %s\n", hostname)
	} else {
		fmt.Printf("os.Hostname failed: %s", err)
	}

	if username[0:len(hostname)] == hostname {
		username = username[len(hostname):]
	}
	if username[0] == '\\' {
		username = username[1:]
	}

	fmt.Printf("username=%s\n", username)
	fmt.Printf("email: <%s@%s>\n", username, hostname)

	const ORIG_OFFSET = 40

	blanks := fmt.Sprintf("%*s", 40, "")
	fmt.Printf("len:%d '%s'\n", len(blanks), blanks)

	buf := make([]byte, 256)
	tagline := fmt.Sprintf("%s %d\x00", "commit", len(buf[40:]))
	if tagline[len(tagline)-1] != 0 {
		fmt.Printf("expected zero byte at end of slice")
	}
	fmt.Printf("len=%d tagline='%s'\n", len(tagline), tagline)
	offset := ORIG_OFFSET - len(tagline)

	n := copy(buf[offset:], tagline)
	if n + offset != ORIG_OFFSET {
		fmt.Fprintf(os.Stderr, "Failed! %d+%d != %d\n", offset, n, ORIG_OFFSET)
	} else {
		fmt.Printf("Success! %d+%d != %d\n", offset, n, ORIG_OFFSET)
	}

	for L := 1; L < 20; L++ {
		size := (62+L+8) & ^7
		fmt.Printf("L=%d size=%d\n", L, size)
	}
}
