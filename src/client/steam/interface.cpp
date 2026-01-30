#include "pch.h"

#include "interface.h"

namespace steam
{
    interface::interface() : interface(nullptr)
    {
    }

    interface::interface(void* interface_ptr) : interface_ptr_(static_cast<void***>(interface_ptr))
    {
    }

    interface::operator bool() const
    {
        return this->interface_ptr_ != nullptr;
    }

    void* interface::find_method(const std::string& name)
    {
        const auto method_entry = this->methods_.find(name);
        if (method_entry != this->methods_.end())
        {
            return method_entry->second;
        }

        return this->search_method(name);
    }

    void* interface::search_method(const std::string& name)
    {
        if (!utils::memory::is_bad_read_ptr(this->interface_ptr_))
        {
            auto vftbl = *this->interface_ptr_;

            while (!utils::memory::is_bad_read_ptr(vftbl) && !utils::memory::is_bad_code_ptr(*vftbl))
            {
                const auto ptr = *vftbl;
                const auto result = this->analyze_method(ptr);
                if (!result.empty())
                {
#if 1
                    std::stringstream ss;
                    ss << "###### search_method result: " << result << std::endl;
                    OutputDebugString(ss.str().c_str());
#endif

                    this->methods_[result] = ptr;

                    if (result == name)
                    {
                        return ptr;
                    }
                }

                ++vftbl;
            }
        }

        return {};
    }

    std::string interface::analyze_method(const void* method_ptr)
    {
        if (utils::memory::is_bad_code_ptr(method_ptr))
            return {};

        ud_t ud;
        ud_init(&ud);
        ud_set_mode(&ud, 32);
        ud_set_pc(&ud, reinterpret_cast<uint64_t>(method_ptr));
        ud_set_input_buffer(&ud, static_cast<const uint8_t*>(method_ptr), INT32_MAX);

        while (true)
        {
            ud_disassemble(&ud);

            // Stop at RET
            if (ud_insn_mnemonic(&ud) == UD_Iret)
                break;

            // Look for PUSH imm32 instructions
            if (ud_insn_mnemonic(&ud) == UD_Ipush)
            {
                const auto* operand = ud_insn_opr(&ud, 0);
                if (operand && operand->type == UD_OP_IMM && operand->size == 32)
                {
                    auto* ptr = reinterpret_cast<char*>(operand->lval.udword);
                    if (!utils::memory::is_bad_read_ptr(ptr) &&
                        utils::memory::is_rdata_ptr(ptr))
                    {
                        return ptr; // Found a valid string
                    }
                }
            }

            // Stop at INT3
            if (*reinterpret_cast<unsigned char*>(ud.pc) == 0xCC)
                break;
        }

        return {};
    }
}