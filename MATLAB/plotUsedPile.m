function [UsedPile] = plotUsedPile(edgePile, edgePassed)
    % plotUsedPile
    load dataGenArgs.mat args
    UsedPile = figure("Name", "UsedPile");
    ax = gca;
    m = length(edgePassed);   % 路径的长度
    p = args.P;             % 通道数量
    
    hold on;
    % 调整颜色方案
    func = @(x) colorspace('RGB->RGB',x);
    colors = distinguishable_colors(args.T + 1,'w', func);
    
    for i = 1 : m
        x = [i - 1, i, i, i - 1];   % 定义X轴的范围
        
        y = zeros(1, 4);        % 定义每个条带的Y轴位置
        edgeID = edgePassed(i); % 当前边的编号
        edgeID = edgeID + 1;    % edgeID原本从0开始，在matlab环境下，需要做出适配
        for j = 1 : p

            y(1, 1 : 2) = j - 1;
            y(1, 3 : 4) = j;
            % 使用fill函数填充颜色
            if edgePile(edgeID, 3 + j) == -1
                fill(x, y, colors(1, :));
            else
                fill(x, y, colors(edgePile(edgeID, 3 + j) + 2, :));
            end
            % 添加标记
            text(i - 0.5, j - 0.5, num2str(edgePile(edgeID, 3 + j)), 'HorizontalAlignment', 'center', 'Color', 'w');

        end
    end
    
    ax.XTick = (1 : m) - 0.5;
    ax.YTick = (1 : p) - 0.5;
    ax.XTickLabel = edgePassed;
    ax.YTickLabel = 1 : p;
    ax.XLim = [0 m];
    ax.YLim = [0 p];
    
    % 设置图表标题和坐标轴标签
    xlabel('Edge ID', 'FontSize', 13);
    ylabel('Pile ID', 'FontSize', 13);
    
    % 显示图表
    hold off;

end