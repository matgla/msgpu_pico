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

#include "mode/program.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace msgpu::mode
{

TEST(ProgramShould, AssignShaders)
{
    Program sut;
    EXPECT_THAT(sut.vertex_shader(), ::testing::IsNull());
    EXPECT_THAT(sut.pixel_shader(), ::testing::IsNull());

    msos::dl::LoadedModule m1, m2;
    Module module_a{
        .type   = ModuleType::VertexShader,
        .module = &m1,
    };

    Module module_b{
        .type   = ModuleType::PixelShader,
        .module = &m2,
    };

    sut.assign_module(module_a);
    sut.assign_module(module_b);

    EXPECT_THAT(sut.vertex_shader(), ::testing::Pointer(&m1));
    EXPECT_THAT(sut.pixel_shader(), ::testing::Pointer(&m2));
}

TEST(ProgramShould, ReturnsIdOfParameter)
{
    Program sut;
    uint8_t id = sut.get_named_parameter_id("Param1");
    EXPECT_EQ(id, 0);

    uint8_t id2 = sut.get_named_parameter_id("Param2");
    EXPECT_EQ(id2, 1);

    EXPECT_EQ(0, sut.get_named_parameter_id("Param1"));
}

TEST(ProgramShould, ReturnNameOfParameter)
{
    Program sut;
    EXPECT_THAT(sut.get_parameter_name(0), ::testing::StrCaseEq(""));
    const uint8_t id  = sut.get_named_parameter_id("p1");
    const uint8_t id2 = sut.get_named_parameter_id("p2");
    EXPECT_THAT(sut.get_parameter_name(id), ::testing::StrCaseEq("p1"));
    EXPECT_THAT(sut.get_parameter_name(id2), ::testing::StrCaseEq("p2"));
}

TEST(ProgramShould, DeleteParameterById)
{
    Program sut;
    const uint8_t id = sut.get_named_parameter_id("Par1");
    EXPECT_THAT(sut.get_parameter_name(id), ::testing::StrCaseEq("Par1"));
    sut.delete_parameter_by_id(id);
    EXPECT_THAT(sut.get_parameter_name(id), ::testing::StrCaseEq(""));
}

TEST(ProgramShould, DeleteParameterByName)
{
    Program sut;
    uint8_t id = sut.get_named_parameter_id("Par1");
    EXPECT_THAT(sut.get_parameter_name(id), ::testing::StrCaseEq("Par1"));
    sut.delete_parameter_by_name("Par1");
    EXPECT_THAT(sut.get_parameter_name(id), ::testing::StrCaseEq(""));
}

} // namespace msgpu::mode
