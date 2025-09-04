#include "test.hpp"
#include "utils.hpp"

using namespace boost::ut;
using namespace saucer::tests;

suite<"smartview"> smartview_suite = []
{
    "expose/evaluate"_test_async = [](saucer::smartview<> &webview)
    {
        webview.set_url("https://codeberg.org/saucer/saucer");

        expect(webview.evaluate<int>("10 + 5").get() == 15);
        expect(webview.evaluate<std::string>("'Hello' + ' ' + 'World'").get() == "Hello World");

        expect(webview.evaluate<int>("{} + {}", 1, 2).get() == 3);
        expect(webview.evaluate<std::string>("{} + {}", "C++", "23").get() == "C++23");
        expect(webview.evaluate<std::array<int, 2>>("Array.of({})", saucer::make_args(1, 2)).get() == std::array<int, 2>{1, 2});

        webview.expose("test1", [](int value) { return value; });
        webview.expose("test2", [](std::string value) -> std::expected<int, std::string> { return std::unexpected{std::move(value)}; });

        static const auto string = saucer::tests::random_string(10);

        expect(eq(webview.evaluate<int>("await saucer.exposed.test1({})", 10).get(), 10));
        expect(webview.evaluate<std::string>("await saucer.exposed.test2({}).then(() => {{}}, err => err)", string).get() == string);

        webview.expose("test3",
                       [](int value, const saucer::executor<int, std::string> &exec)
                       {
                           const auto &[resolve, reject] = exec;

                           if (value < 0)
                           {
                               return reject("negative");
                           }

                           return resolve(value);
                       });

        expect(eq(webview.evaluate<int>("await saucer.exposed.test3({})", 10).get(), 10));
        expect(webview.evaluate<std::string>("await saucer.exposed.test3({}).then(() => {{}}, err => err)", -10).get() == "negative");

        webview.expose("test4",
                       [](int value, const saucer::executor<int, std::string> &exec)
                       {
                           std::thread thread{[=]
                                              {
                                                  const auto &[resolve, reject] = exec;

                                                  std::this_thread::sleep_for(std::chrono::seconds(1));

                                                  if (value < 0)
                                                  {
                                                      return reject("negative");
                                                  }

                                                  return resolve(value);
                                              }};

                           thread.detach();
                       });

        expect(eq(webview.evaluate<int>("await saucer.exposed.test4({})", 10).get(), 10));
        expect(webview.evaluate<std::string>("await saucer.exposed.test4({}).then(() => {{}}, err => err)", -10).get() == "negative");
    };
};
