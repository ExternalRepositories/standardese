// Copyright (C) 2016 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef STANDARDESE_MD_CUSTOM_HPP_INCLUDED
#define STANDARDESE_MD_CUSTOM_HPP_INCLUDED

#include <standardese/md_entity.hpp>

namespace standardese
{
    class md_section final : public md_container
    {
    public:
        static md_entity::type get_entity_type() STANDARDESE_NOEXCEPT
        {
            return md_entity::section_t;
        }

        static md_ptr<md_section> make(const md_entity& parent, const std::string& section_text);

        const char* get_section_text() const STANDARDESE_NOEXCEPT;

        void set_section_text(const std::string& text);

    protected:
        md_entity_ptr do_clone(const md_entity* parent) const override;

    private:
        md_section(const md_entity& parent, const std::string& section_text);

        friend detail::md_ptr_access;
    };
} // namespace standardese

#endif // STANDARDESE_MD_CUSTOM_HPP_INCLUDED