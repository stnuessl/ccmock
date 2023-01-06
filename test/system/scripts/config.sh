# 
# Copyright (C) 2023  Steffen Nuessle
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
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

function validate_clang_section() {
    local result="$1"

    if ! echo "${result}" | grep "/usr/share" > /dev/null; then
        echo "error: \"ResourceDirectory\": invalid value"
        exit 1
    fi

    if ! echo "${result}" | grep -e "-DMY_MACRO1" > /dev/null; then
        echo "error: \"-DMY_MACRO1\": missing value"
        exit 1
    fi

    if ! echo "${result}" | grep -e "-DMY_MACRO2" > /dev/null; then
        echo "error: \"-DMY_MACRO2\": missing value"
        exit 1
    fi

    if ! echo "${result}" | grep -e "-fno-color-diagnostics" > /dev/null; then
        echo "error: \"-fno-color-diagnostics\": missing value" 
        exit 1
    fi

    if ! echo "${result}" | grep -e "-pedantic" > /dev/null; then
        echo "error: \"-pedantic\": missing value" 
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

result="$(${CCMOCK} --config=clang.yaml --dump-config)"
validate_clang_section "${result}"

result="$( \
    CCMOCK_CONFIG=gmock.yaml \
    ${CCMOCK} --config=clang.yaml,mocking.yaml --dump-config \
)"

validate_gmock_section "${result}"
validate_mocking_section "${result}"
validate_clang_section "${result}"
