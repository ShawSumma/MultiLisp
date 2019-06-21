import Sys;
import String;
import sys.io.File;
import haxe.rtti.Rtti;
import StringTools;

class Var {
    private var val : Dynamic;
    public function new(val : Dynamic) {
        this.val = val;
    }
    public function get() : Dynamic {
        return this.val;
    }
    public function set(val : Dynamic) {
        this.val = val;
    }
}

enum OpType {
    PASS;
    INIT;
    PUSH;
    POP;
    CALL;
    STORE;
    LOAD;
    FUNC;
    CAPTURE;
    RET;
    NAME;
    SPACE;
    JUMP;
    JUMPFALSE;
}

class Opcode {
    public var op: OpType;
    public var val: Int;
    public function new(op: String, ?val : Int) {
        if (op == 'pass') this.op = PASS;
        if (op == 'init')  this.op = INIT;
        if (op == 'push')  this.op = PUSH;
        if (op == 'pop')  this.op = POP;
        if (op == 'call')  this.op = CALL;
        if (op == 'store')  this.op = STORE;
        if (op == 'load')  this.op = LOAD;
        if (op == 'func')  this.op = FUNC;
        if (op == 'capture')  this.op = CAPTURE;
        if (op == 'ret')  this.op = RET;
        if (op == 'space')  this.op = SPACE;
        if (op == 'jump')  this.op = JUMP;
        if (op == 'jumpfalse')  this.op = JUMPFALSE;
        this.val = val;
    }
}

class Place {
    public var cap : Array<Var>;
    public var place : Int;
    public function new(place : Int, cap : Array<Var>) {
        this.place = place;
        this.cap = cap;
    }
}

class Callable {
    public var isplace : Bool;
    public var call : Dynamic;
    public function new(call : Dynamic, ?isplace : Bool) {
        this.call = call;
        if (isplace == null) {
            this.isplace = false;
        }
        else {
            this.isplace = isplace;
        }
    }
}

class Lisp {
    var locals : Array<Array<Var>>;
    var captures : Array<Var>;
    var stack : Array<Dynamic>;
    var globals : Map<String, Dynamic>;
    var pl : Int;
    var rets : Array<Int>;
    var values : Array<Dynamic>;
    public function new() {
        this.locals = [[]];
        this.stack = [];
        this.captures = [];
        this.rets = [];
        this.values = [];
    }
    public function initglobals() {
        globals = new Map<String, Dynamic>();
        function b_print(state: Lisp, args: Array<Dynamic>) : Dynamic{
            for (i in args) {
                Sys.print(i);
            }
            return null;
        }
        function b_println(state: Lisp, args: Array<Dynamic>) : Dynamic{
            b_print(state, args);
            Sys.println("");
            return null;
        }
        function b_add(state: Lisp, args: Array<Dynamic>) : Dynamic{
            var pl = 0;
            var ret : Float = 0;
            while (pl < args.length) {
                ret += args[pl];
                pl += 1;
            }
            return ret;
        }
        function b_sub(state: Lisp, args: Array<Dynamic>) : Dynamic{
            if (args.length == 1) {
                return -args[0];
            }
            var pl = 1;
            var ret : Float = args[0];
            while (pl < args.length) {
                ret -= args[pl];
                pl += 1;
            }
            return ret;
        }
        function b_mul(state: Lisp, args: Array<Dynamic>) : Dynamic{
            var pl = 0;
            var ret : Float = 1;
            while (pl < args.length) {
                ret *= args[pl];
                pl += 1;
            }
            return ret;
        }
        function b_div(state: Lisp, args: Array<Dynamic>) : Dynamic{
            if (args.length == 1) {
                return 1/args[0];
            }
            var pl = 1;
            var ret : Float = args[0];
            while (pl < args.length) {
                ret /= args[pl];
                pl += 1;
            }
            return ret;
        }
        function b_lt(state: Lisp, args: Array<Dynamic>) : Dynamic{
            var pl = 1;
            var fst : Float = args[0];
            while (pl < args.length) {
                if (fst < args[pl]) {
                    fst = args[pl];
                }
                else {
                    return false;
                }
                pl = pl + 1;
            }
            return true;
        }
        function b_gt(state: Lisp, args: Array<Dynamic>) : Dynamic{
            var pl = 1;
            var fst : Float = args[0];
            while (pl < args.length) {
                if (fst > args[pl]) {
                    fst = args[pl];
                }
                else {
                    return false;
                }
                pl = pl + 1;
            }
            return true;
        }
        function b_lte(state: Lisp, args: Array<Dynamic>) : Dynamic{
            var pl = 1;
            var fst : Float = args[0];
            while (pl < args.length) {
                if (fst <= args[pl]) {
                    fst = args[pl];
                }
                else {
                    return false;
                }
                pl = pl + 1;
            }
            return true;
        }
        function b_gte(state: Lisp, args: Array<Dynamic>) : Dynamic{
            var pl = 1;
            var fst : Float = args[0];
            while (pl < args.length) {
                if (fst >= args[pl]) {
                    fst = args[pl];
                }
                else {
                    return false;
                }
                pl = pl + 1;
            }
            return true;
        }
        function b_eq(state: Lisp, args: Array<Dynamic>) : Dynamic{
            var pl = 1;
            var fst : Float = args[0];
            while (pl < args.length) {
                if (fst == args[pl]) {
                    fst = args[pl];
                }
                else {
                    return false;
                }
                pl = pl + 1;
            }
            return true;
        }
        function b_neq(state: Lisp, args: Array<Dynamic>) : Dynamic{
            for (i in 0...args.length) {
                for (j in (i+1)...args.length) {
                    if (args[i] == args[j]) {
                        return false;
                    }
                }
            }
            return true;
        }
        function b_time(state: Lisp, args: Array<Dynamic>) : Dynamic{
            return Sys.time();
        }
        function b_timer(state: Lisp, args: Array<Dynamic>) : Dynamic{
            var otime = Sys.time();
            return new Callable(function(state: Lisp, args: Array<Dynamic>) : Dynamic{
                return Sys.time()-otime;
            });
        }
        globals["println"] = new Callable(b_println);
        globals["print"] = new Callable(b_print);
        globals['+'] = new Callable(b_add);
        globals['-'] = new Callable(b_sub);
        globals['*'] = new Callable(b_sub);
        globals['/'] = new Callable(b_sub);
        globals['='] = new Callable(b_eq);
        globals['!='] = new Callable(b_neq);
        globals['<'] = new Callable(b_lt);
        globals['>'] = new Callable(b_gt);
        globals['<='] = new Callable(b_lte);
        globals['>='] = new Callable(b_gte);
        globals['time'] = new Callable(b_time);
        globals['timer'] = new Callable(b_timer);
    }
    public function runfile(filename: String) {
        if (filename == null) {
            Sys.println("error: vm needs a bytecode filename as argument");
            Sys.exit(1);
        }
        var code: String = sys.io.File.getContent(filename);
        var lines: Array<String> = code.split("\n");
        var retcodes: Array<Opcode> = [];
        for (line in lines) {
            if (line != '') {
                var opcode = line.split(' ')[0];
                if (opcode == 'init') {
                    initglobals();
                    retcodes.push(new Opcode('pass'));
                }
                else if (opcode == 'name') {
                    locals[0].push(new Var(globals[line.substring(16)]));
                    retcodes.push(new Opcode('pass'));
                }
                else if (opcode == 'space') {
                    locals[0].push(new Var(null));
                    retcodes.push(new Opcode('pass'));
                }
                else if (opcode == 'str') {
                    retcodes.push(new Opcode('push', values.length));
                    var str : Array<String> = [];
                    var place : Int = 16;
                    var end : Int = line.length;
                    while (place < end) {
                        var cur : String = line.charAt(place);
                        if (cur == '\\') {
                            place += 1;
                            cur = line.charAt(place);
                            if (cur == '\\') {
                                str.push('\\');
                            }
                            else if (cur == 'n') {
                                str.push('\n');
                            }
                            else if (cur == 'r') {
                                str.push('\r');
                            }
                            else if (cur == 't') {
                                str.push('\t');
                            }
                            else if (cur == 's') {
                                str.push(' ');
                            }
                            else if (cur == '0') {
                                place = place + 1;
                                cur = line.substr(place, 2);
                                var num : Int = (cur.charCodeAt(0)-48)*8 + cur.charCodeAt(1)-48;
                                str.push(String.fromCharCode(num));
                                place = place + 1;
                            }
                        } 
                        else {
                            str.push(cur);
                        }
                        place += 1;
                    }
                    values.push(str.join(''));
                }
                else if (opcode == 'push') {
                    retcodes.push(new Opcode('push', values.length));
                    values.push(Std.parseFloat(line.substring(16)));
                }
                else if (opcode == 'arg') {
                    retcodes.push(new Opcode('arg', line.substring(16) == 'True' ? 1 : 0));
                }
                else {
                    if (line.length == 16) {
                        retcodes.push(new Opcode(opcode));
                    }
                    else {
                        retcodes.push(new Opcode(opcode, Std.parseInt(line.substring(16))));
                    }
                }
            }
        }
        runcode(retcodes);
    }
    public function runcode(codes: Array<Opcode>) {
        pl = 0;
        while (pl < codes.length) {
            var cur : Opcode = codes[pl];
            switch (cur.op) {
                case LOAD:
                    stack.push(locals[locals.length-1][cur.val].get());
                case PUSH:
                    stack.push(values[cur.val]);
                case POP:
                    stack.pop();
                case CALL:
                    var localargs : Array<Dynamic>  = [];
                    var argc : Int = cur.val;
                    var rem = argc;
                    for (i in 0...argc) {
                        localargs.push(stack[stack.length-rem]);
                        rem -= 1;
                    }
                    for (i in 0...argc) {
                        stack.pop();
                    }
                    var func : Callable = stack.pop();
                    if (func.isplace) {
                        locals.push([]);
                        var newpl : Place = func.call;
                        rets.push(pl);
                        pl = newpl.place;
                        for (i in localargs) {
                            locals[locals.length-1].push(new Var(i));
                        }
                        for (i in newpl.cap) {
                            locals[locals.length-1].push(i);
                        }
                    }
                    else {
                        var rawfunc = func.call;
                        stack.push(rawfunc(this, localargs));
                    }
                case STORE:
                    locals[locals.length-1][cur.val].set(stack[stack.length-1]);
                case CAPTURE:
                    captures.push(locals[locals.length-1][cur.val]);
                case FUNC:
                    stack.push(new Callable(new Place(pl, captures), true));
                    captures = [];
                    pl = cur.val;
                case JUMPFALSE:
                    var cond = stack.pop();
                    if (cond == null || cond == false) {
                        pl = cur.val;
                    }
                case JUMP:
                    pl = cur.val;
                case RET:
                    pl = rets.pop();
                    locals.pop();
                case pass:
            }
            pl += 1;
        }
    }
}

class Main {
    static public function main():Void {
        var x = [1, 2, 3];
        var args = Sys.args();
        var interp = new Lisp();
        interp.runfile(args[0]);
    }
}