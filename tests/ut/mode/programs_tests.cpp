/*
 *   Copyright (c) 2021 Mateusz Stadnik

 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <cstdint>
#include <fstream>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "mode/programs.hpp"
#include "msos/dynamic_linker/dynamic_linker.hpp"
#include "msos/dynamic_linker/environment.hpp"

extern "C"
{
    int input_a;
    int input_b;
    int *output;
}

namespace msgpu::mode
{

class ProgramsShould : public ::testing::Test
{
  public:
    ProgramsShould()
    {
    }

    const msos::dl::LoadedModule *load_module(std::string_view name)
    {
        msos::dl::Environment env{};
        std::vector<uint8_t> data;

        std::ifstream s;
        s.open(name.data(), std::ios::binary | std::ios::ate);
        if (!s.is_open())
        {
            std::cerr << "Can't open file: " << name << std::endl;
            std::abort();
        }
        std::size_t file_size = s.tellg();
        std::cout << "Filesize: " << file_size << std::endl;
        s.seekg(0, s.beg);

        data.resize(file_size);
        s.read(reinterpret_cast<char *>(data.data()), file_size);
        s.close();

        eul::error::error_code ec;
        return linker_.load_module(std::span<const uint8_t>(data), msos::dl::LoadingModeCopyData,
                                   env, ec);
    }

  protected:
    Programs sut_;
    msos::dl::DynamicLinker linker_;
};

TEST_F(ProgramsShould, AllocateModule)
{
    for (std::size_t i = 0; i < MAX_MODULES_LIST_SIZE; ++i)
    {
        EXPECT_EQ(i, sut_.allocate_module());
    }
    EXPECT_EQ(std::numeric_limits<uint8_t>::max(), sut_.allocate_module());
    EXPECT_EQ(std::numeric_limits<uint8_t>::max(), sut_.allocate_module());
}

TEST_F(ProgramsShould, AllocatePrograms)
{
    for (std::size_t i = 0; i < MAX_PROGRAM_LIST_SIZE; ++i)
    {
        EXPECT_EQ(i, sut_.allocate_program());
    }
    EXPECT_EQ(std::numeric_limits<uint8_t>::max(), sut_.allocate_program());
    EXPECT_EQ(std::numeric_limits<uint8_t>::max(), sut_.allocate_program());
}

TEST_F(ProgramsShould, AddShaders)
{
    uint8_t vertex_shader_id = sut_.allocate_module();
    const auto *module       = load_module(VERTEX_SHADER_PATH);
    sut_.add_vertex_shader(vertex_shader_id, module);

    uint8_t program_id = sut_.allocate_program();
    EXPECT_EQ(sut_.get(program_id)->vertex_shader_, nullptr);
    EXPECT_EQ(sut_.get(program_id)->pixel_shader_, nullptr);
    sut_.assign_module(program_id, vertex_shader_id);
    input_a = 10;
    input_b = 25;
    int answer;
    output = &answer;
    sut_.get(program_id)->vertex_shader_->execute();

    EXPECT_EQ(answer, input_a + input_b);

    const auto *fragment             = load_module(FRAGMENT_SHADER_PATH);
    const uint8_t fragment_shader_id = sut_.allocate_module();

    sut_.add_fragment_shader(fragment_shader_id, fragment);
    sut_.assign_module(program_id, fragment_shader_id);

    input_a = 120;
    input_b = 8;
    sut_.get(program_id)->pixel_shader_->execute();
    EXPECT_EQ(answer, input_a * input_b);
}

} // namespace msgpu::mode
