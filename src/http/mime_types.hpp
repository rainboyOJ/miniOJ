#pragma once
#include <string>
#include <string_view>

namespace http {

    struct mime_types {
        using mapping =  struct { const char* extension; const char* mime_type; };
        constexpr static const mapping mappings[] = {
            { "gif", "image/gif" },
            { "htm", "text/html" },
            { "html", "text/html" },
            { "jpg", "image/jpeg" },
            { "png", "image/png" },
            { 0, 0 } // Marks end of list.
        };

        static std::string extension_to_type(std::string_view extension)
        {
            if( extension[0] == '.') extension.remove_prefix(1);

            for (const mapping* m = mappings; m->extension; ++m)
            {
                if (m->extension == extension)
                {
                    return m->mime_type;
                }
            }
            return "text/plain";
        }
    }; // struct mime_types end


} //end namespace http
