#include <algorithm>
#include <bitset>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

#include <kibble/argparse/argparse.h>
#include <kibble/assert/assert.h>
#include <kibble/logger/dispatcher.h>
#include <kibble/logger/logger.h>
#include <kibble/logger/sink.h>
#include <kibble/string/string.h>

using namespace kb;
namespace fs = std::filesystem;

void init_logger()
{
    KLOGGER_START();
    KLOGGER(create_channel("thext", 3));
    KLOGGER(attach_all("console_sink", std::make_unique<klog::ConsoleSink>()));
    KLOGGER(set_backtrace_on_error(false));
}

void show_error_and_die(ap::ArgParse& parser)
{
    for(const auto& msg : parser.get_errors())
        KLOGW("thext") << msg << std::endl;

    KLOG("thext", 1) << parser.usage() << std::endl;
    exit(0);
}

bool validate(const std::string& input, size_t divisor, const std::string& allowed)
{
    if(input.size() % divisor != 0)
    {
        KLOGE("thext") << "Invalid input string: number of symbols should be a multiple of " << divisor << std::endl;
        return false;
    }
    auto pos = input.find_first_not_of(allowed);
    if(pos != std::string::npos)
    {
        KLOGE("thext") << "Invalid input string: symbol at column " << pos + 1 << " is not in range." << std::endl;
        KLOGR("thext") << input << std::endl;
        for(size_t ii = 0; ii < pos; ++ii)
        {
            KLOGR("thext") << '-';
        }
        KLOGR("thext") << '^' << std::endl;
        return false;
    }
    return true;
}

std::string ascii2hex(const std::string& input)
{
    std::stringstream ss;
    ss << std::hex;
    for(size_t ii = 0; ii < input.size(); ++ii)
    {
        ss << int(input[ii]);
        if(ii < input.size() - 1)
            ss << ' ';
    }
    return ss.str();
}

std::string ascii2bin(const std::string& input)
{
    std::stringstream ss;
    for(size_t ii = 0; ii < input.size(); ++ii)
    {
        ss << std::bitset<8>(size_t(input[ii]));
        if(ii < input.size() - 1)
            ss << ' ';
    }
    return ss.str();
}

unsigned char hexval(unsigned char c)
{
    if('0' <= c && c <= '9')
        return c - '0';
    else if('a' <= c && c <= 'f')
        return c - 'a' + 10;
    else if('A' <= c && c <= 'F')
        return c - 'A' + 10;
    else
        abort();
}

std::string hex2ascii(const std::string& input)
{
    std::string out;
    out.reserve(input.length() / 2);
    for(std::string::const_iterator p = input.begin(); p != input.end(); p++)
    {
        unsigned char c = hexval(static_cast<unsigned char>(*p));
        p++;
        if(p == input.end())
            break;
        c = static_cast<unsigned char>(c << 4) + hexval(static_cast<unsigned char>(*p));
        out.push_back(char(c));
    }
    return out;
}

std::string bin2ascii(const std::string& input)
{
    std::string out;
    std::stringstream ss(input);
    while(ss.good())
    {
        std::bitset<8> bits;
        ss >> bits;
        if(bits.to_ulong() != 0)
        	out += char(bits.to_ulong());
    }
    return out;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    init_logger();

    ap::ArgParse parser("thext", "0.1");
    parser.set_log_output([](const std::string& str) { KLOG("thext", 1) << str << std::endl; });
    parser.set_exit_on_special_command(true);
    parser.add_flag('a', "iascii", "ASCII input, no line break");
    const auto& ihex = parser.add_flag('x', "ihex", "Hexadecimal input, no spaces");
    const auto& ibinary = parser.add_flag('n', "ibinary", "Binary input, no spaces");
    const auto& ibase64 = parser.add_flag('b', "ib64", "Base64 input");
    const auto& oascii = parser.add_flag('A', "oascii", "ASCII output");
    const auto& ohex = parser.add_flag('X', "ohex", "Hexadecimal output");
    const auto& obinary = parser.add_flag('N', "obinary", "Binary output");
    const auto& obase64 = parser.add_flag('B', "ob64", "Base64 output");
    parser.set_flags_exclusive({'a', 'x', 'n', 'b'});

    bool success = parser.parse(argc, argv);

    if(!success)
        show_error_and_die(parser);

    // Get input
    std::string line_in;
    std::getline(std::cin, line_in);

    // First, convert to ASCII
    std::string as_ASCII;
    if(ihex())
    {
        line_in.erase(std::remove(line_in.begin(), line_in.end(), ' '), line_in.end());
        if(!validate(line_in, 2, "0123456789abcdefABCDEF"))
            exit(0);
        as_ASCII = hex2ascii(line_in);
    }
    else if(ibinary())
    {
        line_in.erase(std::remove(line_in.begin(), line_in.end(), ' '), line_in.end());
        if(!validate(line_in, 8, "01"))
            exit(0);
        as_ASCII = bin2ascii(line_in);
    }
    else if(ibase64())
    {
        if(!validate(line_in, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="))
            exit(0);
        as_ASCII = su::base64_decode(line_in);
    }
    else
        as_ASCII = line_in;

    // If multiple targets are active, format output
    size_t num_targets = oascii() + ohex() + obinary() + obase64();
    bool format_output = false;
    bool output_all = (num_targets == 0);
    if(output_all || num_targets > 1)
        format_output = true;

    // Helper lambda to format output
    auto do_print = [](bool format, const std::string& label, const std::string& input,
                       std::function<std::string(const std::string&)> convert) {
        if(format)
        {
            KLOGR("thext") << label << WCB(0, 153, 0);
        }
        KLOGR("thext") << convert(input) << WCB(0) << std::endl;
    };

    // Run converters
    if(oascii() || output_all)
        do_print(format_output, "ASCII: ", as_ASCII, [](const std::string& s) { return s; });
    if(ohex() || output_all)
        do_print(format_output, "HEX:   ", as_ASCII, ascii2hex);
    if(obinary() || output_all)
        do_print(format_output, "BIN:   ", as_ASCII, ascii2bin);
    if(obase64() || output_all)
        do_print(format_output, "B64:   ", as_ASCII, su::base64_encode);

    return 0;
}