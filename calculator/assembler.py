#!/usr/bin/env python3
import re
import sys

is_first_pass = True
labels = {}
tokenized_lines = []

def is_reg(token):
    try:
        reg_bin(token)
        return True
    except ValueError:
        return False

def reg_bin(token):
    try:
        return {
            'r1': '00',
            'r2': '01',
            'r3': '10',
            'rstack': '11'
        }[token]
    except KeyError:
        raise ValueError

def is_imm(token, bits):
    try:
        imm_bin(token, bits)
        return True
    except ValueError:
        return False

def to_bits(i, bits):
    assert i >= 0
    b = ''
    while i > 0:
        b += '1' if i % 2 == 1 else '0'
        i = i >> 1
    if len(b) > bits:
        raise ValueError
    b += '0' * (bits - len(b))
    return b

def imm_bin(token, bits):
    if token[0] == '@':
        if is_first_pass:
            return '0' * bits
        else:
            return to_bits(labels[token[1:]], bits)
    else:
        return to_bits(int(token), bits)

def do_a_pass():
    global is_first_pass
    binary = []
    for line, tokens in tokenized_lines:
        ip_in_bits = sum(map(len, binary))
        assert ip_in_bits % 10 == 0
        ip = ip_in_bits // 10
        if tokens == []:
            pass
        elif (len(tokens) == 1) and tokens[0].endswith(':'):
            label = tokens[0][:-1]
            if is_first_pass:
                assert label not in labels, label
                labels[label] = ip
        elif tokens[0] == 'copy':
            assert len(tokens) == 3
            assert is_reg(tokens[2])
            if is_imm(tokens[1], 10):
                b = '0101' + '0' + ('0' * 3) + imm_bin(tokens[1], 10) + reg_bin(tokens[2])
                assert len(b) == 20
                binary.append(b)
            elif is_reg(tokens[1]):
                b = '0101' + '1' + ('0' * 11) + reg_bin(tokens[1]) + reg_bin(tokens[2])
                assert len(b) == 20
                binary.append(b)
            else:
                assert False, line
        elif tokens[0] == 'add':
            assert len(tokens) == 3
            assert is_reg(tokens[2])
            if is_imm(tokens[1], 10):
                b = '0110' + '0' + ('0' * 3) + imm_bin(tokens[1], 10) + reg_bin(tokens[2])
                assert len(b) == 20
                binary.append(b)
            elif is_reg(tokens[1]):
                b = '0110' + '1' + ('0' * 11) + reg_bin(tokens[1]) + reg_bin(tokens[2])
                assert len(b) == 20
                binary.append(b)
            else:
                assert False, line
        elif tokens[0] == 'get_pin':
            assert len(tokens) == 2
            b = '0010' + imm_bin(tokens[1], 6)
            assert len(b) == 10
            binary.append(b)
        elif tokens[0].startswith('jump'):
            assert len(tokens) == 2
            ty_bin = imm_bin(str({
                'jump': 0,
                'jump_if_1': 1
            }[tokens[0]]), 6)
            b = '0000' + ty_bin + imm_bin(tokens[1], 10)
            assert len(b) == 20
            binary.append(b)
        elif tokens[0].startswith('call'):
            assert len(tokens) == 2
            b = '0001' + ('0' * 6) + imm_bin(tokens[1], 10)
            assert len(b) == 20
            binary.append(b)
        elif tokens[0].startswith('set_pin'):
            assert len(tokens) == 2
            b = '0011' + imm_bin(tokens[1], 6)
            assert len(b) == 10
            binary.append(b)
        elif tokens[0].startswith('clear_pin'):
            assert len(tokens) == 2
            b = '0100' + imm_bin(tokens[1], 6)
            assert len(b) == 10
            binary.append(b)
        elif tokens[0] == 'return':
            assert len(tokens) == 1
            b = '0111' + ('0' * 6)
            assert len(b) == 10
            binary.append(b)
        elif tokens[0] == 'imm':
            assert len(tokens) == 2
            binary.append(imm_bin(tokens[1], 10))
        else:
            assert False, line
    is_first_pass = False
    return binary

with open(sys.argv[1]) as f:
    for line in f:
        # Get rid of the comment.
        line = line.split('#')[0]
        tokens = list(filter(lambda s: s != '', re.split(r'\s+', line)))
        tokenized_lines.append((line, tokens))

# First pass: figure out label offsets.
do_a_pass()

# Second pass: actually output the binary.
for b in do_a_pass():
    print(b)
