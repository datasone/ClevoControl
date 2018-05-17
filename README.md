# ClevoControl
Control keyboard backlight and fan policy for Clevo Hackintosh
This project only supports clevo laptops with full-color backlight support. However, you can easily modify it to fit in 8-color backlight keyboards.
I have only tested it on my Clevo P650RS so I won't promise it will work, but it should.

# How to use
Directly calling WMBB method in WMI device will lead to a kernel panic. I have no idea about why it's happening, so you should add following code into your DSDT:

```
Device (SMCD)
{
    Name (_HID, EisaId ("PNP0C02"))
    Name (_CID, "MON00000")
    Method (WMIB, 3, Serialized)
    {
        \_SB.WMI.WMBB(Arg0, Arg1, Arg2)
    }
}
```

put it above `Device (EC)`, but below `Scope (\_SB.PCI0.LPCB)`

You may already have SMCD device in your DSDT, then please ensure one of your \_HID or \_CID is "MON0000" or "MON00000" and add WMIB method into it.

After you edit your DSDT, install ClevoControl.kext to /L/E (injecting it using Clover cause kernel panic) and use ClevoKBFanControl (a small command-line program) to control settings.

Show usage by `ClevoKBFanControl -h`.

This project uses [args](https://github.com/Taywee/args) for parsing command-line arguments. And the WMI operate codes used in this project comes from [clover-xsm-wmi](https://github.com/sonnym/clevo-xsm-wmi) and reverse engineering Clevo Control Center by myself.

# Some Extra information
## Fan Speed
Edit DSDT, for my laptop with 3 fans, add these to SMCD device (it uses B1B2 method, add it if you don't have it in your DSDT):

```
Name (TACH, Package (0x06)
    {
        "CPU Fan", "FAN0",
        "GPU Fan #1", "FAN1",
        "GPU Fan #2", "FAN2"
    }) // Define fan names

Method (FAN0, 0, Serialized)
{
    If (\_SB.PCI0.LPCB.EC.ECOK)
    {
        Local0 = B1B2(\_SB.PCI0.LPCB.EC.FC01, \_SB.PCI0.LPCB.EC.FC00)
        If (Local0 <= 0)
        {
            Return (0)
        }
        Local0 = 2156220 / Local0
        Return (Local0)
    }
    Return (0)
}

Method (FAN1, 0, Serialized)
{
    If (\_SB.PCI0.LPCB.EC.ECOK)
    {
        Local0 = B1B2(\_SB.PCI0.LPCB.EC.FG01, \_SB.PCI0.LPCB.EC.FG00)
        If (Local0 <= 0)
        {
            Return (0)
        }
        Local0 = 2156220 / Local0
        Return (Local0)
    }
    Return (0)
}

Method (FAN2, 0, Serialized)
{
    If (\_SB.PCI0.LPCB.EC.ECOK)
    {
        Local0 = B1B2(\_SB.PCI0.LPCB.EC.FG11, \_SB.PCI0.LPCB.EC.FG10)
        If (Local0 <= 0)
        {
            Return (0)
        }
        Local0 = 2156220 / Local0
        Return (Local0)
    }
    Return (0)
}
```

The B1B2 method can be added with MaciASL:
```
into method label B1B2 remove_entry;
into definitionblock code_regex . insert
begin
Method (B1B2, 2, NotSerialized) { Return(Or(Arg0, ShiftLeft(Arg1, 8))) }\n
end;
```

If you only have 2 or 1 fan, delete FAN2 or FAN2 & FAN1, respectively.

Then find for EmbeddedControl Field in your DSDT, search for EmbeddedControl in your DSDT and you will see something like this:

```
OperationRegion (EC81, EmbeddedControl, Zero, 0xFF)
Field (EC81, ByteAcc, Lock, Preserve)
{
...
}
```

Add these in the Field method as first. As mentioned above delete extra values. e.g. delete FG10 and FG11 if you don't have two GPU fans.

```
Offset (0xD0),
FC00,   8,
FC01,   8,    // CPU Fan Speed
FG00,   8,
FG01,   8,    // GPU Fan0 Speed
FG10,   8,
FG11,   8,    // GPU Fan1 Speed
```

Last, edit FakeSMC.kext/Contents/Info.plist, change the data in FNum from 00 to number of your fans.
