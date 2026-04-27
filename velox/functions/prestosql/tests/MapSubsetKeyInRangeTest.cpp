/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "velox/functions/prestosql/tests/utils/FunctionBaseTest.h"

using namespace facebook::velox::test;

namespace facebook::velox::functions {
namespace {

class MapSubsetKeyInRangeTest : public test::FunctionBaseTest {};

TEST_F(MapSubsetKeyInRangeTest, basicBigintKey) {
  auto inputMap = makeMapVectorFromJson<int64_t, int32_t>({
      "{1:10, 2:20, 3:30, 4:40, 5:50}",
      "{7:70, 10:100, 14:140, 20:200}",
      "{}",
  });

  auto expected = makeMapVectorFromJson<int64_t, int32_t>({
      "{2:20, 3:30, 4:40, 5:50}",
      "{7:70, 10:100, 14:140}",
      "{}",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(2 as bigint), cast(14 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, boundaryInclusivity) {
  auto inputMap = makeMapVectorFromJson<int64_t, int32_t>({
      "{1:10, 2:20, 3:30, 4:40, 5:50}",
  });

  // Bounds are inclusive: keys 1 and 5 should be included.
  auto expected = makeMapVectorFromJson<int64_t, int32_t>({
      "{1:10, 2:20, 3:30, 4:40, 5:50}",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(1 as bigint), cast(5 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, allKeysInRange) {
  auto inputMap = makeMapVectorFromJson<int64_t, int32_t>({
      "{1:10, 2:20, 3:30}",
      "{5:50}",
  });

  auto expected = makeMapVectorFromJson<int64_t, int32_t>({
      "{1:10, 2:20, 3:30}",
      "{5:50}",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(0 as bigint), cast(100 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, allKeysOutOfRange) {
  auto inputMap = makeMapVectorFromJson<int64_t, int32_t>({
      "{1:10, 2:20, 3:30}",
      "{100:1000, 200:2000}",
  });

  auto expected = makeMapVectorFromJson<int64_t, int32_t>({
      "{}",
      "{}",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(50 as bigint), cast(60 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, lowGreaterThanHigh) {
  auto inputMap = makeMapVectorFromJson<int64_t, int32_t>({
      "{1:10, 2:20, 3:30, 4:40, 5:50}",
      "{7:70, 14:140}",
  });

  // When low > high, the result is always an empty map.
  auto expected = makeMapVectorFromJson<int64_t, int32_t>({
      "{}",
      "{}",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(14 as bigint), cast(7 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, emptyMap) {
  auto inputMap =
      makeMapVectorFromJson<int64_t, int32_t>({"{}", "{}", "{1:10}"});

  auto expected =
      makeMapVectorFromJson<int64_t, int32_t>({"{}", "{}", "{1:10}"});

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(0 as bigint), cast(100 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, nullMap) {
  auto inputMap = makeNullableMapVector<int64_t, int32_t>({
      std::nullopt,
      {{{1, 10}, {2, 20}, {3, 30}}},
      std::nullopt,
  });

  auto expected = makeNullableMapVector<int64_t, int32_t>({
      std::nullopt,
      {{{2, 20}}},
      std::nullopt,
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(2 as bigint), cast(2 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, nullValuesPreserved) {
  auto inputMap = makeNullableMapVector<int64_t, int32_t>({
      {{{1, 10}, {2, std::nullopt}, {3, 30}, {4, std::nullopt}, {5, 50}}},
  });

  auto expected = makeNullableMapVector<int64_t, int32_t>({
      {{{2, std::nullopt}, {3, 30}, {4, std::nullopt}}},
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(2 as bigint), cast(4 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, varcharKey) {
  auto inputMap = makeMapVectorFromJson<std::string, int32_t>({
      R"({"apple":1, "banana":2, "cherry":3, "date":4, "eggplant":5})",
      R"({"x":10, "y":20, "z":30})",
  });

  auto expected = makeMapVectorFromJson<std::string, int32_t>({
      R"({"banana":2, "cherry":3, "date":4})",
      R"({})",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, 'banana', 'date')",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, varcharKeyBoundaryInclusive) {
  auto inputMap = makeMapVectorFromJson<std::string, int32_t>({
      R"({"apple":1, "banana":2, "cherry":3})",
  });

  auto expected = makeMapVectorFromJson<std::string, int32_t>({
      R"({"apple":1, "banana":2, "cherry":3})",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, 'apple', 'cherry')",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, intKeyTypes) {
  // Verify int32 (integer) keys work via primitive specialization.
  auto inputMap = makeMapVectorFromJson<int32_t, int32_t>({
      "{1:10, 2:20, 3:30, 4:40}",
  });

  auto expected = makeMapVectorFromJson<int32_t, int32_t>({
      "{2:20, 3:30}",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(2 as integer), cast(3 as integer))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, doubleKey) {
  auto inputMap = makeMapVectorFromJson<double, int32_t>({
      "{1.5:1, 2.5:2, 3.5:3, 4.5:4}",
  });

  auto expected = makeMapVectorFromJson<double, int32_t>({
      "{2.5:2, 3.5:3}",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, 2.0, 4.0)", makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, sameLowAndHigh) {
  auto inputMap = makeMapVectorFromJson<int64_t, int32_t>({
      "{1:10, 2:20, 3:30}",
      "{5:50}",
  });

  auto expected = makeMapVectorFromJson<int64_t, int32_t>({
      "{2:20}",
      "{}",
  });

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(2 as bigint), cast(2 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

TEST_F(MapSubsetKeyInRangeTest, complexValues) {
  // Map<bigint, array<int>>: verify generic value type is preserved.
  auto inputMap = makeMapVector(
      {0, 3},
      makeFlatVector<int64_t>({1, 2, 3, 10, 20}),
      makeArrayVectorFromJson<int32_t>({
          "[1, 2]",
          "[3, 4]",
          "[5, 6]",
          "[100]",
          "[200]",
      }));

  auto expected = makeMapVector(
      {0, 2},
      makeFlatVector<int64_t>({2, 3, 10}),
      makeArrayVectorFromJson<int32_t>({
          "[3, 4]",
          "[5, 6]",
          "[100]",
      }));

  auto result = evaluate(
      "map_subset_key_in_range(c0, cast(2 as bigint), cast(10 as bigint))",
      makeRowVector({inputMap}));
  assertEqualVectors(expected, result);
}

} // namespace
} // namespace facebook::velox::functions
