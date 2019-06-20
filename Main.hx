import Sys;
import String;
import sys.io.File;
import haxe.rtti.Rtti;

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

class Opcode {
    public var op: String;
    public var val: Int;
    public function new(op: String, ?val : Int) {
        this.op = op;
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

class Interp {
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
        function b_print(state: Interp, args: Array<Dynamic>) : Dynamic{
            for (i in args) {
                Sys.print(i);
            }
            return null;
        }
        function b_println(state: Interp, args: Array<Dynamic>) : Dynamic{
            b_print(state, args);
            Sys.println("");
            return null;
        }
        function b_add(state: Interp, args: Array<Dynamic>) : Dynamic{
            var pl = 0;
            var ret : Float = 0;
            while (pl < args.length) {
                ret += args[pl];
                pl += 1;
            }
            return ret;
        }
        function b_sub(state: Interp, args: Array<Dynamic>) : Dynamic{
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
        function b_mul(state: Interp, args: Array<Dynamic>) : Dynamic{
            var pl = 0;
            var ret : Float = 1;
            while (pl < args.length) {
                ret *= args[pl];
                pl += 1;
            }
            return ret;
        }
        function b_div(state: Interp, args: Array<Dynamic>) : Dynamic{
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
        function b_lt(state: Interp, args: Array<Dynamic>) : Dynamic{
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
        function b_neq(state: Interp, args: Array<Dynamic>) : Dynamic{
            for (i in 0...args.length) {
                for (j in (i+1)...args.length) {
                    if (args[i] == args[j]) {
                        return false;
                    }
                }
            }
            return true;
        }
        globals["print"] = new Callable(b_print);
        globals['+'] = new Callable(b_add);
        globals['-'] = new Callable(b_sub);
        globals['*'] = new Callable(b_sub);
        globals['/'] = new Callable(b_sub);
        globals['<'] = new Callable(b_lt);
        globals['!='] = new Callable(b_neq);
        globals["println"] = new Callable(b_println);
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
                    values.push(line.substring(16));
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
            if (cur.op == 'load') {
                stack.push(locals[locals.length-1][cur.val].get());
            }
            else if (cur.op == 'push') {
                stack.push(values[cur.val]);
            }
            else if (cur.op == 'pop') {
                stack.pop();
            }
            else if (cur.op == 'call') {
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
            }
            else if (cur.op == 'store') {
                locals[locals.length-1][cur.val].set(stack[stack.length-1]);
            } 
            else if (cur.op == 'capture') {
                captures.push(locals[locals.length-1][cur.val]);
            }
            else if (cur.op == 'func') {
                stack.push(new Callable(new Place(pl, captures), true));
                captures = [];
                pl = cur.val;
            }
            else if (cur.op == 'jumpfalse') {
                var cond = stack.pop();
                if (cond == null || cond == false) {
                    pl = cur.val;
                }
            }
            else if (cur.op == 'jump') {
                pl = cur.val;                
            }
            else if (cur.op == 'ret') {
                pl = rets.pop();
                locals.pop();
            }
            else if (cur.op == 'pass') {}
            else{
                Sys.println(cur.op);
                Sys.exit(1);
            }
            pl += 1;
        }
    }
}

class Main {
    static public function main():Void {
        var x = [1, 2, 3];
        var args = Sys.args();
        var interp = new Interp();
        interp.runfile(args[0]);
    }
}