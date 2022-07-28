.kp.foo:{[x;y]
    2*x+1
    };

.z.pw:{[user;pwd]
    show user;
    show pwd;
    :1b;
    };

.z.po:{[hdl] show hdl;};

.z.pg:{[x]
    show "evaluating sync message";
    value x
    };

.z.ps:{[x]
    show "evaluating async message";
    show x;
    };

.kp.makeArr:{[x]
    size:"j"$(2 xexp 32)-15;
    val:"x"$x;
    arr:"x"$(size#val);
    arr[0]:0x09;
    arr[1]:0x08;
    arr[2]:0x07;
    arr
    };