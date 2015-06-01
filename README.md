# dranzer
Dranzer is a tool that enables users to examine effective techniques for fuzz testing ActiveX controls

# Dranzer 1.9 User’s Guide

Dan Plakosh and Will Dormann

April 16, 2009

# Overview

ActiveX and COM vulnerabilities have been getting much attention lately. ActiveX allows a web browser to use software components installed on a Windows machine. Scripting technologies can allow an attacker to control the memory contents of a machine. By combining scripting and ActiveX, an attacker can take advantage of flaws in COM objects, which may allow execution of arbitrary code, information disclosure, or other security violations.

Dranzer is a tool that can detect flaws in COM objects.

# Target Audience

Dranzer can be useful to various groups of people:

1. Software developers can test COM objects as they are being developed. By testing during the software development process, developers can prevent vulnerabilities before the software is released to the public.
2. Software vendors can detect and fix flaws in their COM objects before attackers discover the flaws.
3. System administrators can assess the security of their systems with respect to COM objects.

# ActiveX Vulnerability Classes
## I. COM objects that crash Internet Explorer upon instantiation.

Some COM objects will crash Internet Explorer just by being referenced in a web page. The COM object may not have been intended for a web browser, but Internet Explorer will attempt to instantiate any COM object that is referenced in an <OBJECT> tag, regardless of whether the object is a traditional ActiveX control. Certain COM objects will cause Internet Explorer to crash in a manner that attackers can exploit to execute arbitrary code.

More information about this class of vulnerabilities is available in “Multiple COM objects cause memory corruption in Microsoft Internet Explorer” ([http://www.kb.cert.org/vuls/id/959049](http://www.kb.cert.org/vuls/id/959049)).

## II. COM objects that fail to properly validate input.

Attackers can use a script within a web page to control COM objects that are marked “Safe for scripting.” This script would call methods or access properties of the COM object. Attackers can control COM objects that are marked “Safe for initialization” by using <PARAM> tags in a web page. If the COM object fails to properly validate input, such as enforcing a maximum size for a string parameter, an attacker may be able to pass specially crafted parameters that would cause the object to crash Internet Explorer in a way that the attacker could exploit.

An example of this type of vulnerability is available in “RealPlayer ActiveX control contains buffer overflow in ‘ShowPreferences’” ([http://www.kb.cert.org/vuls/id/698390](http://www.kb.cert.org/vuls/id/698390)).

## III. COM objects that do not restrict access to its methods.

Unless special restrictions are in place, any web page can call the methods provided by safe-for-scripting COM objects. Attackers may be able to take advantage of some of these methods to disclose information unintentionally; they may even be able to download and execute applications.

An example of this type of vulnerability is available in “Microsoft Log Sink Class ActiveX control incorrectly marked ‘safe for scripting’” ([http://www.kb.cert.org/vuls/id/165022](http://www.kb.cert.org/vuls/id/165022)).

# Dranzer system requirements

Operating system: Windows Vista, XP or Windows 2000. Windows XP is preferred.

Browser: Internet Explorer with ActiveX enabled for the Internet Zone (IE 6 and earlier) and Local Intranet Zone (IE 7 or later)

Note: We do not recommend running Dranzer on production systems. The process of testing COM objects essentially involves executing pieces of code that exist on a system, and this may have adverse effects in a test environment.

# Running Dranzer

    C:\Program Files\Dranzer\Dranzer\Release>Dranzer.exe
    Execution mode not specified use -g,-k,-l,-b, or -p
    Usage: Dranzer.exe <options>
    
    Options:
    -o <outputfile> - Output Filename
    -i <inputfile> - Use input file CLSID list
    -d <notestfile> - Use don't test CLSID List
    -g - Generate base COM list
    -k - Generate Kill Bit COM list
    -l - Generate Interface Listings
    -b - Load In Browser (IE)
    -t - Test Interfaces Properties and Methods
    -p - Test PARAMS (PropertyBag) in Internet Explorer
    -s - Test PARAMS (Binary Scan) in Internet Explorer
    -n - Print COM object information
    -v - Print out version information

Dranzer has the ability to test all three of the COM vulnerability classes described above.

## Class I: (-b)

With the `-b` option, Dranzer will check for COM objects that cause Internet Explorer to crash upon their instantiation.

Example usage:

    Dranzer.exe -o testmachine_crashie.txt -b

## Class II: (-t, -p, -s)

With the `-t` option, Dranzer will check for COM objects that fail to properly validate input to methods.

Example usage:

    Dranzer.exe -o testmachine_report.txt -t

With the `-p` and `-s` options, Dranzer will check for COM objects that fail to properly validate input to initialization parameters. Dranzer will check for available parameters by using either the IPropertyBag interface or by scanning the binary file, respectively.

Example usage:

    Dranzer.exe -o testmachine_param_bag.txt -p
    Dranzer.exe -o testmachine_param_scan.txt -s

## Class III: (-l)

With the `-l` option, Dranzer will enumerate the methods and properties of COM objects that are marked "safe for scripting."

Example usage:

    Dranzer.exe –o testmachine_methods.txt -l

## Using baselines and exception lists

The `-g` option can be used to create a `baseline` list of COM objects installed on a machine. This capability can be useful for determining what COM objects an application provides. For example:

1) Create a baseline snapshot for the machine being used in the test:

    Dranzer.exe -o baseline.txt -g

2) Install software package

3) Run Dranzer with the `-d` option to exclude the COM objects in the baseline:

    Dranzer -d baseline.txt -o myapp_crashie.txt -b

Alternatively, you can execute the `alltests.bat` script to run all Dranzer tests for COM objects that do not exist in the `baseline.txt` file. Rather than using the command line listed in step 3, simply run `alltests.bat`. This will test only the new COM objects that were installed with the software package in step 2. Included in the Dranzer installation are the files `xpprosp2.txt` and `vista.txt`. These files contain a list of COM objects that come with Windows XP Professional SP2 and Windows Vista, respectively. You can use these files as a starting point for testing COM objects on a system if you were unable to generate a baseline before installing the software.

## Using input lists

The `-i` option can be used if you wish to provide a list of the COM objects that you would like to be tested.

Example usage:

    Dranzer.exe -i mycomobjects.txt -o myobjects_test.txt -t

This command will test the methods of the COM objects listed in the `mycomobjects.txt` input file. 

# Dranzer output examples
## Top-level errors

In the Dranzer output, these errors are listed at the beginning of each failed COM object. Possible errors include the following:

    ERROR - Access violation (0xc0000005)

This error indicates a crash as the result of accessing an invalid area of memory.

    ERROR - Buffer Overrun Fault (0xfffffff3)

This error is usually indicative of a buffer overflow that is caught by Microsoft’s /GS buffer security check. The /GS buffer overflow protection can make it more difficult to exploit the vulnerability. However, depending on what the vulnerable control’s code does, and whether the /SAFESEH flag was used, an attacker can often bypass this protection. Note that Dranzer can produce this error as a false positive if the control uses the same error code for other errors. Also note that Dranzer will not clearly indicate which method caused the buffer overrun fault error code, but it will be the last method or property listed in the Dranzer output.

    ERROR - COM Object Exception Occurred (0xfffffff9)

The COM object has generated an exception, which is usually the result of a memory access violation.

    ERROR - Internet Explorer Crashed (0xfffffff7)

The HTML test case caused IE to crash. Use the relevant HTML file in the Dranzer directory to reproduce the crash.

    ERROR - COM Object Operation Hung (0xffffffff)

The COM object has not responded within the time allowed. Certain COM objects can hang when a web page attempts to use them; others can have specific methods that will cause the hang. With either of these situations, the errors are not normally exploitable, but in certain cases a buffer overflow or other error can cause memory corruption that will lead to a hang. You need to further investigate these errors by testing other input values, if possible, to determine if this is the case.

## Method-level errors

In the Dranzer output, Method-level errors are listed for specific methods or properties. The following are example errors:

    *****************************
    *** Access Violation ***
    *****************************
    Invoked Property Get - ObjectName::short PropName()
    Access violation at 0x6E205539 :Bad read on 0x00000000

This is a null pointer dereference access violation as the result of attempting to refer to the “PropName” property. It is less likely to be exploitable because of the null pointer and also because the property takes no parameters. However, keep in mind that the Dranzer testing is sequential, so a crash in a specific method or parameter may be the result of the actions that took place before (above) it.

    *****************************
    *** Access Violation ***
    *****************************
    Invoked Method - ObjectName::VARIANT_BOOL MethodName()
    Access violation at 0x7C91142E :Bad read on 0x78787878

This indicates that a buffer overflow has taken place, with the memory address being the value specified in a buffer value (lowercase ‘x’ being 0x78 in ASCII). Depending on what the COM object is doing with the value that is being read from that location, this flaw has a high probability that an attacker can exploit it to execute arbitrary code. Note that this is another case in which the method that triggers the access violation is not the one that accepts user input. The buffer overflow likely took place in a previous parameter or method operation, but what is experienced can be considered a “second-order” flaw because the symptoms of the flaw are not immediately seen.

    *****************************
    *** Access Violation ***
    *****************************
    Invoked Method - ObjectName::long MethodName([in] BSTR
    ParameterName<"xxxx.....{10240}">)
    Access violation at 0x78787878 :Bad read on 0x78787878

This indicates a buffer overflow vulnerability that is trivially exploitable. Two identical memory addresses indicate that the COM object is trying to execute code at the specified address. In this case, it’s the hex value of the characters in the parameter specified for the method.

    *****************************
    *** Access Violation ***
    *****************************
    Invoked Property Get - ObjectName::long PropertyName(long ParameterName<-1>)
    Access violation at 0x6338092E :Bad read on 0xFFFFFFFF

This is an example of a possible integer overflow. Depending on how the memory address can be affected by various parameter values, the flaw may be exploitable.

**Note:** If there is a top-level error reported, but no method-level violation is present, look at the last method or parameter that Dranzer tested in the output. This last entry is likely the one that caused the SEH to be overwritten or caused /GS protection to trigger. Both of these cases will prevent a method-level violation from being reported. Vulnerabilities in which the SEH is overwritten as the result of a buffer overflow are usually trivially exploitable.

The behavior where the test process is terminated unexpectedly can cause important flaws to be overlooked. This can happen when a method-level violation is reported, but also the last method tested causes one of the exceptions described above. Dranzer may not report the method-level exception in the output. To work around this situation, use Dranzer to retest COM objects after all reported violations are fixed in the code and pay special attention to the last method listed in Dranzer test reports.

## Class III: (-l)

This class of vulnerabilities requires the most analysis to determine if there is a problem. Look for method names that an attacker could leverage. The following are some examples of dangerous methods:

    ShellExecute(BSTR)
    Reboot()
    DownloadFile(BSTR,BSTR)
    HttpPOST(BSTR,BSTR,BSTR)
    GetUserName()

If the COM object does not restrict which web addresses can use the methods, attackers may be able to use the COM object to their advantage.

# Other COM fuzzers
## axfuzz 
http://sourceforge.net/projects/axfuzz

axfuzz was the inspiration for creating Dranzer. With the axfuzz package, you must use a combination of the axenum and axfuzz tools to fuzz test an entire system. When the testing tools crash, the tools must manually be restarted at a specific control after the one that caused the crash, and crash details are not included in the reports.

## COMRaider
http://labs.idefense.com/labs-software.php?show=20

COMRaider is a graphical tool for fuzz testing a single COM object. Crash details are included, which can help determine which COM flaws may be exploitable. Due to the program design, a high level of user interaction is required, and the tests take a long time to complete.

## AxMan
http://metasploit.com/users/hdm/tools/axman

AxMan is a browser-based method fuzz tester. AxMan is designed to fuzz test all of the COM objects installed on a system, and the test process involves multiple steps. Once a crash is encountered, the test process must be manually restarted to begin again using a control that exists in the list after the one that caused the crash. COM objects that display dialogs or present the Internet Explorer information bar will require a user to manually click those items to continue the test process. AxMan does not appear to support Internet Explorer 7.

## COM fuzzer comparison

A single COM object that was known to be vulnerable was used to compare the fuzz test tools:

Feature | Dranzer | axfuzz | COMRaider | AxMan 
 -- | -------- | ------ | --------- | ----- 
Time to test object | 1 sec. | 4 sec. | 140 sec. | 660 sec.
Exceptions found | 3 | 2 | 3 | 1
Vulnerability classes covered ||||
Class I | Yes | No | No | No
Class II (methods) | Yes | Yes | Yes | Yes
Class II (params) | Yes | No | No | No
Class III | Yes | No | No | No
Output | Text | Text | Database | None
User interaction required | None | Medium | High | Very high
Test multiple objects | Yes | Yes | Yes | Yes 
Test sequences | Yes | Yes | No | Yes
Crash details reported | Yes | No | Yes | No
Multiple test values | No | No | Yes | Yes

# References

Microsoft ActiveX security resources:

* *Designing Secure ActiveX Controls* http://msdn.microsoft.com/en-us/library/aa752035.aspx
* *ActiveX Security: Improvements and Best Practices* http://msdn.microsoft.com/en-us/library/bb250471.aspx
* *How to stop an ActiveX control from running in Internet Explorer* http://support.microsoft.com/kb/240797

*Results of the CERT/CC Security in ActiveX Workshop* http://www.cert.org/reports/activeX_report.pdf

Vulnerability notes:

* *Multiple COM objects cause memory corruption in Microsoft Internet Explorer* http://www.kb.cert.org/vuls/id/959049
* *RealPlayer ActiveX control contains buffer overflow in ‘ShowPreferences’* http://www.kb.cert.org/vuls/id/698390
* *Microsoft Log Sink Class ActiveX control incorrectly marked ‘safe for scripting’* http://www.kb.cert.org/vuls/id/165022

Other COM fuzzers:

* *axfuzz* http://sourceforge.net/projects/axfuzz
* *iDefense Labs Fuzzing Software Tools* http://labs.idefense.com/software/fuzzing.php#more_comraider
* *AxMan* http://metasploit.com/users/hdm/tools/axman

