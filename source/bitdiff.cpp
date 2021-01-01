/* Binary string diff utility
 * The input to this utility is a formatted text file written in this fashion:
 * 		[Category A]
 * 		0000111101101001010010100110111000110110
 * 		0000111101000101001011010110111010110100
 * 		0000111111010100100010010110111010110010
 * 		0000111111101001000110000110111000111011
 * 		[Category B]
 * 		0000111100011010001111010111100100111110
 * 		0000111111011000011001010111100110111101
 * 		0000111110010011101001110111100100110010
 * 		0000111100010110011011000111100101110000
 *
 * This utility will detect inter-category and intra-category similarities in
 * the binary strings and highlight the corresponding patterns.
 *
 * - All examples must be of the same length.
 * - There can be as many categories as you like.
 *
 * Example (run from the build directory):
 * > ../bin/bitdiff ../data/bitdiff.txt
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

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
    KLOGGER(create_channel("bitdiff", 3));
    KLOGGER(attach_all("console_sink", std::make_unique<klog::ConsoleSink>()));
    KLOGGER(set_backtrace_on_error(false));
}

void show_error_and_die(ap::ArgParse& parser)
{
    for(const auto& msg : parser.get_errors())
        KLOGW("bitdiff") << msg << std::endl;

    KLOG("bitdiff", 1) << parser.usage() << std::endl;
    exit(0);
}

struct binary_string
{
    std::vector<bool> str;

    binary_string() = default;
    binary_string(const std::string& str) { initialize(str); }

    inline void initialize(const std::string& input)
    {
        for(char c : input)
            str.push_back((c == '0') ? false : true);
    }

    inline void resize(size_t size) { str.resize(size); }
    inline size_t size() const { return str.size(); }
    inline bool operator[](size_t idx) const { return str[idx]; }
    friend std::ostream& operator<<(std::ostream& stream, const binary_string& other);
    binary_string operator&(const binary_string& other) const;

    static inline binary_string compare(const binary_string& A, const binary_string& B)
    {
        K_ASSERT(A.size() == B.size(), "Cannot compare binary strings of different lengths.");
        binary_string ret;
        ret.resize(A.size());
        for(size_t ii = 0; ii < A.size(); ++ii)
            ret.str[ii] = !(A[ii] ^ B[ii]);
        return ret;
    }
};

binary_string binary_string::operator&(const binary_string& other) const
{
    K_ASSERT(size() == other.size(), "Cannot AND binary strings of different lengths.");
    binary_string ret;
    ret.resize(size());
    for(size_t ii = 0; ii < size(); ++ii)
        ret.str[ii] = str[ii] && other.str[ii];
    return ret;
}

std::ostream& operator<<(std::ostream& stream, const binary_string& other)
{
    for(size_t ii = 0; ii < other.size(); ++ii)
        stream << other[ii];
    return stream;
}

class Examples
{
public:
    Examples() = default;

    bool insert(const std::string& str);

    inline const auto& sims() const { return sims_; }
    inline const auto& operator[](size_t idx) const { return example_[idx]; }
    inline size_t size() const { return example_.size(); }

private:
    std::vector<binary_string> example_;
    binary_string sims_;
    size_t example_size_ = 0;
};

bool Examples::insert(const std::string& str)
{
    if(example_size_ == 0)
    {
        example_size_ = str.size();
        sims_.resize(example_size_);
        for(size_t ii = 0; ii < example_size_; ++ii)
            sims_.str[ii] = true;
    }
    if(str.size() != example_size_)
        return false;

    example_.emplace_back(str);
    if(size() > 1)
        sims_ = sims_ & binary_string::compare(example_[example_.size() - 2], example_[example_.size() - 1]);

    return true;
}

int main(int argc, char** argv)
{
    init_logger();

    ap::ArgParse parser("bitdiff", "0.1");
    parser.set_log_output([](const std::string& str) { KLOG("bitdiff", 1) << str << std::endl; });
    parser.set_exit_on_special_command(true);

    const auto& target =
        parser.add_positional<std::string>("FILE", "File containing classified example binary strings");
    bool success = parser.parse(argc, argv);

    if(!success)
        show_error_and_die(parser);

    KLOGN("bitdiff") << "--------[BITDIFF]--------" << std::endl;
    fs::path filepath(target());
    if(!fs::exists(filepath))
    {
        KLOGE("bitdiff") << "File does not exist:" << std::endl;
        KLOGI << KS_PATH_ << filepath << std::endl;
    }

    size_t current_category = 0;
    std::vector<std::string> categories;
    std::vector<Examples> example_set;

    std::ifstream ifs(filepath);
    std::regex re("\\[(.+)\\]");
    for(std::string line; std::getline(ifs, line);)
    {
        std::smatch sm;
        std::regex_match(line, sm, re);
        if(sm.size() > 1)
        {
            current_category = categories.size();
            categories.push_back(sm[1]);
            example_set.emplace_back();
        }
        else
        {
            auto& exs = example_set.at(current_category);
            if(!exs.insert(line))
            {
                KLOGE("bitdiff") << "All examples must be of the same length." << std::endl;
                return 0;
            }
        }
    }

    binary_string inter_sims;
    inter_sims.resize(example_set[0][0].size());
    for(size_t ii = 0; ii < inter_sims.size(); ++ii)
        inter_sims.str[ii] = true;

    binary_string last_ex = example_set[0][0];
    for(size_t ii = 0; ii < example_set.size(); ++ii)
    {
        const auto& exs = example_set[ii];
        for(size_t jj = 0; jj < exs.size(); ++jj)
        {
            inter_sims = inter_sims & binary_string::compare(last_ex, exs[jj]);
            last_ex = exs[jj];
        }
    }

    for(size_t ii = 0; ii < example_set.size(); ++ii)
    {
        std::string cat = categories[ii];
        su::center(cat, int(example_set[0][0].size()) + 4);
        KLOGR("bitdiff") << KF_(255, 100, 0) << cat << KC_ << std::endl;
        const auto& intra_sims = example_set[ii].sims();
        for(size_t jj = 0; jj < example_set[ii].size(); ++jj)
        {
            const auto& ex = example_set[ii][jj];
            KLOGR("bitdiff") << "[" << jj << "] ";
            for(size_t kk = 0; kk < ex.size(); ++kk)
            {
                if(inter_sims[kk])
                {
                    KLOGR("bitdiff") << KB_(10, 75, 150);
                }
                else if(intra_sims[kk])
                {
                    KLOGR("bitdiff") << KB_(150, 75, 10);
                }
                else
                {
                    KLOGR("bitdiff") << KC_;
                }
                KLOGR("bitdiff") << ex.str[kk];
            }
            KLOGR("bitdiff") << KC_ << std::endl;
        }
    }

    return 0;
}
