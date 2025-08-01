from conan import ConanFile, tools
from conan.tools.cmake import CMake, cmake_layout, CMakeToolchain
from conan.tools.scm import Git
import re

class OpenEawConan(ConanFile):
    name = "openeaw"
    license = "MIT"
    homepage = "https://github.com/OpenEaW/OpenEaW"
    description = "An open-source implementation of the game Empire at War"

    def set_version(self):
        git = Git(self, self.recipe_folder)

        self.version = "0.0.0"
        try:
            GIT_SHORT_HASH_LENGH=12
            latest_tag = git.run("describe --tags --abbrev=0").strip()
            result = re.match(r"v?(\d+.\d+.\d+)", latest_tag)
            if result:
                self.version = "{}".format(result.group(1))
        except Exception:
            pass

        try:
            # Store the short Git version because Conan has a limit of the length of the version string
            self.version += "+{}".format(git.get_commit()[:GIT_SHORT_HASH_LENGH])
            if not git.is_pristine():
                self.version += ".dirty"
        except Exception:
            pass

    settings = "os", "compiler", "build_type", "arch"

    generators = "CMakeDeps"

    def requirements(self):
        self.requires("assimp/[>=5.0 <6.0]")
        self.requires("cxxopts/3.0.0")
        self.requires("diligent-core/api.252009")
        self.requires("fmt/10.1.0")
        self.requires("freetype/[>=2.0 <3.0]")
        self.requires("glfw/[>=3.0 <4.0]")
        self.requires("gsl-lite/0.37.0")
        self.requires("rapidxml/1.13")

    def build_requirements(self):
        self.test_requires("gtest/[>=1.0 <2.0]")

    exports_sources = "CMakeLists.txt", "openglyph/*", "khepri/*", "src/*"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)

        # To enable static analyses tools like clang-tidy
        tc.cache_variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = "ON"

        # Propagate version info into CMake
        version_info = self._parse_version(self.version)
        if version_info:
            tc.variables['OPENEAW_VERSION_MAJOR'] = version_info['major']
            tc.variables['OPENEAW_VERSION_MINOR'] = version_info['minor']
            tc.variables['OPENEAW_VERSION_PATCH'] = version_info['patch']
            tc.variables['OPENEAW_VERSION_COMMIT'] = version_info['commit']
            tc.variables['OPENEAW_VERSION_CLEAN'] = str(version_info['clean']).lower()
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    @staticmethod
    def _parse_version(version):
        result = re.match(r"(\d+).(\d+).(\d+)\+([a-fA-F0-9]+)(\.dirty)?", version)
        if result:
            return {
                'major': int(result.group(1)),
                'minor': int(result.group(2)),
                'patch': int(result.group(3)),
                'commit': result.group(4),
                'clean': (result.group(5) is None)
            }
