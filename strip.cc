#include <iostream>
#include <filesystem>

#include <sourcemeta/jsontoolkit/json.h>
#include <sourcemeta/jsontoolkit/jsonschema.h>

int main(int argc, char* argv[]) {
  if (argc != 2) {
      std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
      return 1;
  }

  std::filesystem::path filepath = argv[1];
  std::cerr << "Stripping: " << filepath << "\n";
  auto schema{sourcemeta::jsontoolkit::from_file(filepath)};

  sourcemeta::jsontoolkit::Frame frame;
  frame.analyse(schema,
    sourcemeta::jsontoolkit::default_schema_walker,
    sourcemeta::jsontoolkit::official_resolver);

  for (const auto &entry : frame.locations()) {
    if (entry.second.type != sourcemeta::jsontoolkit::Frame::LocationType::Subschema &&
        entry.second.type != sourcemeta::jsontoolkit::Frame::LocationType::Resource) {
      continue;
    }

    auto &subschema{get(schema, entry.second.pointer)};

    for (const auto &property : subschema.as_object()) {
      const auto keyword_type{sourcemeta::jsontoolkit::default_schema_walker(
            property.first, frame.vocabularies(entry.second,
              sourcemeta::jsontoolkit::official_resolver))
                  .type};
      if (
          keyword_type == sourcemeta::jsontoolkit::KeywordType::Annotation ||
          keyword_type == sourcemeta::jsontoolkit::KeywordType::Comment ||
          keyword_type == sourcemeta::jsontoolkit::KeywordType::Unknown
      ) {
        subschema.erase(property.first);
      }
    }
  }

  sourcemeta::jsontoolkit::prettify(schema,
      std::cout, sourcemeta::jsontoolkit::schema_format_compare);
  std::cerr << "\n";

  return 0;
}
