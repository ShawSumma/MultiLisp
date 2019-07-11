from walker import Instr

def packs(walk):
    data = []
    instrs = []
    datac = 0
    for pl, i in enumerate(walk.code):
        v = i.vals[0] if len(i.vals) > 0 else 0
        # print(pl,'\t', int(i.kind), '\t', str(i.kind)[5:])
        if i.kind == Instr.Kind.PUSH:
            v = i.vals[0]
            if isinstance(v, int):
                vs = str(v).encode('ascii')
                data.append(b'i')
                data.append(vs)
                data.append(b'e')
            if isinstance(v, float):
                vs = str(v).encode('ascii')
                data.append(b'd')
                data.append(vs)
                data.append(b'e')
            if v is None:
                data.append(b'n')
            if isinstance(v, str):
                vs = v.encode('ascii')
                data.append(b's')
                data.append(str(len(vs)).encode('ascii'))
                data.append(b'e')
                data.append(vs)
            hexv = str(datac).encode('ascii')
            datac += 1
        elif i.kind == Instr.Kind.NAME:
            v = i.vals[0]
            vs = v.encode('ascii')
            data.append(b's')
            data.append(str(len(vs)).encode('ascii'))
            data.append(b'e')
            data.append(vs)
            data.append(b'l')
            hexv = str(datac).encode('ascii')
            datac += 1
        else:
            hexv = str(int(v)).encode('ascii')
        op = str(int(i.kind)).encode('ascii')
        if len(op) == 1:
            op = b'0'+op
        # print(op.decode('ascii'))
        instrs.append(op + hexv)
        instrs.append(b'e')
    return str(len(data)).encode('ascii') + b'e' + b''.join(data) + b':' + str(len(instrs)).encode('ascii') + b'e' + b''.join(instrs)