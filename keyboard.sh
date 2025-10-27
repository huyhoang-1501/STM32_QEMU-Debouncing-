#!/bin/bash
echo "=== TOGGLE LED ==="
(echo "P"; sleep 3; echo "P"; sleep 3; echo "P") | nc localhost 1234