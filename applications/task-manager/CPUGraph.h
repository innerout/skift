#pragma once

#include <libsystem/eventloop/Timer.h>
#include <libsystem/system/System.h>
#include <libsystem/utils/BufferBuilder.h>
#include <libwidget/Widgets.h>

#include "task-manager/TaskModel.h"

class CPUGraph : public Graph
{
private:
    TaskModel *_model;

    Label *_label_average;
    Label *_label_greedy;
    Label *_label_uptime;

    RefPtr<Timer> _graph_timer{};
    RefPtr<Timer> _text_timer{};

public:
    CPUGraph(Widget *parent, TaskModel *model)
        : Graph(parent, 256, COLOR_SEAGREEN),
          _model(model)
    {
        layout(VFLOW(0));
        insets(Insets(8));
        attributes(LAYOUT_FILL);

        auto icon_and_text = new Container(this);
        icon_and_text->layout(HFLOW(4));
        new IconPanel(icon_and_text, Icon::get("memory"));
        new Label(icon_and_text, "Processor");

        auto cpu_filler = new Container(this);
        cpu_filler->attributes(LAYOUT_FILL);

        _label_average = new Label(this, "Average: nil%", Position::RIGHT);
        _label_greedy = new Label(this, "Most greedy: nil", Position::RIGHT);
        _label_uptime = new Label(this, "Uptime: nil", Position::RIGHT);

        _graph_timer = make<Timer>(100, [&]() {
            SystemStatus status = system_get_status();
            record(status.cpu_usage / 100.0);
        });

        _graph_timer->start();

        _text_timer = make<Timer>(1000, [&]() {
            SystemStatus status = system_get_status();
            const char *greedy = task_model_get_greedy_process(_model, 1);

            char buffer_average[50];
            snprintf(buffer_average, 200, "Average: %i%%", (int)(average() * 100.0));
            _label_average->text(buffer_average);

            auto buffer_greedy = buffer_builder_create(24);
            buffer_builder_append_str(buffer_greedy, "Most greedy: ");
            buffer_builder_append_str(buffer_greedy, greedy);
            _label_greedy->text(buffer_builder_intermediate(buffer_greedy));
            buffer_builder_destroy(buffer_greedy);

            ElapsedTime seconds = status.uptime;
            int days = seconds / 86400;
            seconds %= 86400;
            int hours = seconds / 3600;
            seconds %= 3600;
            int minutes = seconds / 60;
            seconds %= 60;

            char buffer_uptime[50];
            snprintf(buffer_uptime, 50, "Uptime: %3d:%02d:%02d:%02d", days, hours, minutes, seconds);
            _label_uptime->text(buffer_uptime);
        });

        _text_timer->start();
    }
};