#
# The MIT License (MIT)
#
# Copyright (c) 2022  Steffen Nuessle
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

set -o pipefail

CCMOCK="$1"

function validate_gmock_section() {
    local result="$1"

    if ! echo "${result}" | grep "MockType:\s*NiceMock" > /dev/null; then
        echo "error: \"MockType\": invalid value"
        exit 1
    fi

    if ! echo "${result}" | grep "MockName:\s*MyMockName" > /dev/null; then
        echo "error: \"MockName\": invalid value"
        exit 1
    fi

    if ! echo "${result}" | grep "MockSuffix:\s*MyMockSuffix" > /dev/null; then
        echo "error: \"MockSuffix\": invalid value"
        exit 1
    fi

    if ! echo "${result}" | grep "WriteMain:\s*false" > /dev/null; then
        echo "error: \"WriteMain\": invalid value"
        exit 1
    fi
}

function validate_mocking_section() {
    local result="$1"

    if ! echo "${result}" | grep "Blacklist:" > /dev/null; then
        echo "error: \"Blacklist\": item not found"
        exit 1
    fi

    if ! echo "${result}" | grep "\-\s\+printf" > /dev/null; then
        echo "error: \"printf\": item not found"
        exit 1
    fi

    if ! echo "${result}" | grep "\-\s\+'std::copy'" > /dev/null; then
        echo "error: \"std::copy\": item not found"
        exit 1
    fi

    if ! echo "${result}" | grep "\-\s\+'__builtin_\*'" > /dev/null; then
        echo "error: \"__builtin_*\": item not found"
        exit 1
    fi

    if ! echo "${result}" | grep "MockBuiltins:\s*true" > /dev/null; then
        echo "error: \"MockBuiltins\": invalid value"
        exit 1
    fi

    if ! echo "${result}" | grep "MockCStdLib:\s*true" > /dev/null; then
        echo "error: \"MockStdlib\": invalid value"
        exit 1
    fi

    if ! echo "${result}" | grep "MockC++StdLib:\s*true" > /dev/null; then
        echo "error: \"MockStdlib\": invalid value"
        exit 1
    fi
}

if [[ ! -f "${CCMOCK}" ]]; then
    echo "error: \${CCMOCK}: invalid argument"
    exit 1
fi

if ! ${CCMOCK} --dump-config -o /dev/null; then
    echo "error: --dump-config failed"
    exit 1
fi

result="$(${CCMOCK} --config=gmock.yaml --dump-config)"
validate_gmock_section "${result}"

result="$(${CCMOCK} --config=mocking.yaml --dump-config)"
validate_mocking_section "${result}"
