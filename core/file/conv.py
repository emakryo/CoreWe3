class ParseError(Exception):
    def __init__(self, s):
        self.s = s

    def __str__(self):
        return str(self.s)


class Instruction:

    def __init__(self, s): # parse line
        self.opcode = None
        self.operand_l = [] # list of operand
        self.blabel_l = [] # list of branch label
        self.ilabel_al = [] # assoc list of immediate label
        t = s.split()
        if len(t) == 0: # empty line
            raise ParseError(s)
        elif len(t) == 1: # only label
            if t[0][0] == ':':
                self.blabel_l.append(t[0])
            else:
                raise ParseError(s)
        elif len(t) == 2 and t[0][0] == '.': # immediate label
            self.ilabel_al.append(t[0], int(t[1], 0))
        else: # instruction
            if t[0][0] == ':':
                self.blabel_l.append(t[0])
                self.opcode = t[1]
                self.operand_l = t[2:]
            else:
                self.opcode = t[0]
                self.operand_l = t[1:]

    def append(self, i): #append to line without instruction
        if (self.opcode == None
            and self.operand_l == None):
            raise Exception
        self.blabel_l.extend(i.blabel_l)
        self.ilabel_al.extend(i.ilabel_al)
        self.opcode = i.opcode
        self.operand_l = i.operand_l

    def conv_pseudo(self, ilabel_d):
        def logical_rshift(x, n):
            return (x % 0x100000000) >> n

        if self.opcode == 'LDI':
            if self.operand_l[1] == '.':
                imm = ilabel_d[self.operand_l[1]]
            else:
                imm = int(self.operand_l[1], 0)
            i1 = Instruction('LDIL ' + self.operand_l[0] + ' ' + str(imm & 2**20-1))
            i1.blabel_l = self.blabel_l
            higher_bits = logical_rshift(imm,20)
            if higher_bits != 0:
                i2 = Instruction('LDIH ' + self.operand_l[0] + ' ' + str(higher_bits))
                return [i1, i2]
            else:
                return [i1]
        else:
            return [self]


    def conv_bin(self, ilabel_d, blabel_d):
        opcode_dic = {'LD'  :0x0,
                      'ST'  :0x1,
                      'LDI' :0x2,
                      'STA' :0x3,
                      'LDIH':0x4,
                      'LDIL':0x5,
                      'ADD' :0x6,
                      'SUB' :0x7,
                      'FNEG':0x8,
                      'ADDI':0x9,
                      'AND' :0xa,
                      'OR'  :0xb,
                      'XOR' :0xc,
                      'SHL' :0xd,
                      'SHR' :0xd,
                      'SHLI':0xf,
                      'SHRI':0x10,
                      'BEQ' :0x11,
                      'BLE' :0x12,
                      'BLT' :0x13,
                      'BFLE':0x14,
                      'JSUB':0x15,
                      'RET' :0x16,
                      'PUSH':0x17,
                      'POP' :0x18,
                      'FADD':0x19,
                      'FMUL':0x1a}
        def reg2int(r):
            if r.startswith('r'):
                return int(r.strip('r'), 0)
            else:
                raise ParseError(r)

        def imm2int(r, bit):
            if r[0] == '.': # immediate label
                n = ilabel_d[r]
            elif r[0] == ':': # branch label
                n = blabel_d[r] - self.line
            else:
                n = int(r, 0)
            return n & (2**bit-1)

        self.binary = opcode_dic[self.opcode] << 26
        if (self.opcode == 'LD' or
            self.opcode == 'ST' or
            self.opcode == 'ADDI' or
            self.opcode == 'SHLI' or
            self.opcode == 'SHRI' or
            self.opcode == 'BEQ' or
            self.opcode == 'BLE' or
            self.opcode == 'BLT' or
            self.opcode == 'BFLE'):
            self.binary |= (reg2int(self.operand_l[0]) & 2**6-1) << 20
            self.binary |= (reg2int(self.operand_l[1]) & 2**6-1) << 14
            self.binary |= imm2int(self.operand_l[2], 14)
        elif (self.opcode == 'LDA' or
              self.opcode == 'STA' or
              self.opcode == 'LDIH' or
              self.opcode == 'LDIL'):
            self.binary |= (reg2int(self.operand_l[0]) & 2**6-1) << 20
            self.binary |= (imm2int(self.operand_l[1], 20) & 2**20-1)
        elif (self.opcode == 'ADD' or
              self.opcode == 'SUB' or
              self.opcode == 'AND' or
              self.opcode == 'OR' or
              self.opcode == 'XOR' or
              self.opcode == 'SHL' or
              self.opcode == 'SHR' or
              self.opcode == 'FADD' or
              self.opcode == 'FMUL'):
            self.binary |= (reg2int(self.operand_l[0]) & 2**6-1) << 20
            self.binary |= (reg2int(self.operand_l[1]) & 2**6-1) << 14
            self.binary |= (reg2int(self.operand_l[2]) & 2**6-1) << 8
        elif (self.opcode == 'FNEG'):
            self.binary |= (reg2int(self.operand_l[0]) & 2**6-1) << 20
            self.binary |= (reg2int(self.operand_l[1]) & 2**6-1) << 14
        elif (self.opcode == 'JSUB'):
            self.binary |= (imm2int(self.operand_l[0], 26))
        elif (self.opcode == 'RET'):
            pass
        elif (self.opcode == 'PUSH' or
              self.opcode == 'POP'):
            self.binary |= (reg2int(self.operand_l[0]) & 2**6-1) << 20;
        else:
            raise ParseError(self.opcode)


class Assembly:
    import sys
    def __init__(self, fin = sys.stdin):
        self.ins_l = [] # list of instructions
        self.blabel_d = dict() # dictionary of branch label
        self.ilabel_d = dict() # dictionary of immediate label
        while(True):
            s = fin.readline();
            if s.strip() == '':
                break
            i = Instruction(s)
            while i.opcode == None:
                i.append(Instruction(fin.readline()));

            self.ins_l.append(i)
            for i in self.ins_l:
                for l, n in i.ilabel_al:
                    self.ilabel_d[l] = n

        tmp = []
        for i in self.ins_l:
            tmp.extend(i.conv_pseudo(self.ilabel_d))
        self.ins_l = tmp

        for n,i in enumerate(self.ins_l):
            i.line = n
            for l in i.blabel_l:
                self.blabel_d[l] = n

        for i in self.ins_l:
            i.conv_bin(self.ilabel_d, self.blabel_d)

    def print_bin(self,fout=sys.stdout):
        import struct
        for i in self.ins_l:
            fout.write(struct.pack('i', i.binary))

    def print_text(self,fout=sys.stdout):
        def print_byte(n):
            fout.write(bin(tmp)[2:].zfill(8))

        for i in self.ins_l:
            tmp = (i.binary >> 24) & 255
            print_byte(tmp)
            tmp = (i.binary >> 16) & 255
            print_byte(tmp)
            tmp = (i.binary >> 8) & 255
            print_byte(tmp)
            tmp = i.binary & 255
            print_byte(tmp)
            fout.write('\n')

    def show_immediate_label(self, fout=sys.stdout):
        for l, n in self.ilabel_d.iteritems():
            fout.write(l+' '+hex(n)+'\n')

    def show_branch_label(self, fout=sys.stdout):
        for l, n in self.blabel_d.iteritems():
            fout.write(l+' '+hex(n)+'\n')

a = Assembly()
a.print_text()
# a.show_branch_label()