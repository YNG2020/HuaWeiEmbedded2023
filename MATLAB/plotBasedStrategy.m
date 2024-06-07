function [basedStrategyVisulization] = plotBasedStrategy(edgePile, edgeStat)
    load flatNodeArray.mat flatNodeArray
    load flatNodeArray.mat nodeArray
    load dataGenArgs.mat args
    load newEdgeIdx.mat newEdgeIdx
    load newEdgeTmpCnt.mat newEdgeTmpCnt
    basedStrategyVisulization = figure("Name", "basedStrategyVisulization");
    set(gcf, 'WindowState', 'maximized');
    p = args.P;             % 通道数量
    maxMultiEdgeID = size(newEdgeIdx, 2);
    func = @(x) colorspace('RGB->RGB',x);
    bestPathID = 1;
    bestPathDis = 100000000;
    breakFlag = 0;

    
    % 显示基本策略中的BFS寻路过程
    fileID = fopen('BFS_Finding_Path.txt', 'r');
    allPath = {};
    lineIndex = 1;
    while ~feof(fileID)
        line = fgetl(fileID); % 读取一行
        if ischar(line)
            if lineIndex == 1
                emphasizeTranID = str2num(line);
                lineIndex = lineIndex + 1;
                continue
            end
            allPath{lineIndex} = str2num(line); % 将字符串转换为数字数组，并存储到 cell 数组中
            lineIndex = lineIndex + 1;
        end
    end
    
    lineIndex = lineIndex - 1;
    rgbpict = zeros(1332, 2560, 3, lineIndex, 'uint8');

    for i = 2 : lineIndex + 1

        if i == lineIndex + 1
            breakFlag = 1;
            i = bestPathID;
        end
        
        backgroundFaceAlpha = 0.2;
        % 调整颜色方案
        colors = distinguishable_colors(args.T + 1,'w', func);
        
        for k = 1 : size(edgePile, 1)
            edgeID = k;
            multiEdgeID = 1;    % 重边编号
            singleEdgeID = 1;   % 不计重边时边的编号
            for j = 1 : maxMultiEdgeID
                if ~isempty(find(newEdgeIdx(:, j) == edgeID - 1, 1))
                    multiEdgeID = j;
                    singleEdgeID = find(newEdgeIdx(:, j) == edgeID - 1, 1);
                end
            end
            startNodeID = edgePile(edgeID, 1) + 1;
            endNodeID = edgePile(edgeID, 2) + 1;
            startX = flatNodeArray{1, startNodeID}.x;
            startY = flatNodeArray{1, startNodeID}.y;
            endX = flatNodeArray{1, endNodeID}.x;
            endY = flatNodeArray{1, endNodeID}.y;
            x = [startX, startX, endX, endX];   % 定义X轴的范围
            y = [startY, startY, endY, endY];   % 定义Y轴的范围
        
            for j = 1 : p
                level = multiEdgeID * (1 / newEdgeTmpCnt(singleEdgeID));
                z = [j - 1 + level - 1 / newEdgeTmpCnt(singleEdgeID), j - 1 + level, j - 1 + level, j - 1 + level - 1 / newEdgeTmpCnt(singleEdgeID)];
                % 使用fill函数填充颜色
                if edgePile(edgeID, 3 + j) == -1
                    fill3(x, y, z, colors(1, :), 'FaceAlpha', 0.05);
                else
                    fill3(x, y, z, colors(edgePile(edgeID, 3 + j) + 2, :), 'FaceAlpha', backgroundFaceAlpha);
                    % 添加标记
                    % text((x(1) + x(4)) / 2, (y(1) + y(4)) / 2, (z(1) + z(2)) / 2, num2str(edgePile(edgeID, 3 + j)),'HorizontalAlignment', 'center', 'Color', 'w');
                end
                hold on;
            end
        end
        ax = gca;
        view(ax, [26.096874999999997, 22.866493548387091]);


        path = allPath{i};
        if length(path) < bestPathDis
            bestPathID = i;
            bestPathDis = length(path);
        end
        pileID = path(1);
        for k = 2 : length(path)
            edgeID = path(k) + 1;
            multiEdgeID = 1;    % 重边编号
            singleEdgeID = 1;   % 不计重边时边的编号
            for j = 1 : maxMultiEdgeID
                if ~isempty(find(newEdgeIdx(:, j) == edgeID - 1, 1))
                    multiEdgeID = j;
                    singleEdgeID = find(newEdgeIdx(:, j) == edgeID - 1, 1);
                end
            end
            startNodeID = edgePile(edgeID, 1) + 1;
            endNodeID = edgePile(edgeID, 2) + 1;
            startX = flatNodeArray{1, startNodeID}.x;
            startY = flatNodeArray{1, startNodeID}.y;
            endX = flatNodeArray{1, endNodeID}.x;
            endY = flatNodeArray{1, endNodeID}.y;
            x = [startX, startX, endX, endX];   % 定义X轴的范围
            y = [startY, startY, endY, endY];   % 定义Y轴的范围
        
            level = multiEdgeID * (1 / newEdgeTmpCnt(singleEdgeID));
            z = [pileID + level - 1 / newEdgeTmpCnt(singleEdgeID), pileID + level, pileID + level, pileID + level - 1 / newEdgeTmpCnt(singleEdgeID)];
            % 使用fill函数填充颜色

            patch = fill3(x, y, z, colors(i, :), 'FaceAlpha', 1);
            if breakFlag
                patch.LineWidth = 2;
                patch.EdgeColor = 'r';
            end
            % 添加标记
            text((x(1) + x(4)) / 2, (y(1) + y(4)) / 2, (z(1) + z(2)) / 2, num2str(emphasizeTranID), 'HorizontalAlignment', 'center', 'Color', 'w');

        end
        for k = 2 : length(path)
            edgeID = path(k) + 1;
            multiEdgeID = 1;    % 重边编号
            singleEdgeID = 1;   % 不计重边时边的编号
            for j = 1 : maxMultiEdgeID
                if ~isempty(find(newEdgeIdx(:, j) == edgeID - 1, 1))
                    multiEdgeID = j;
                    singleEdgeID = find(newEdgeIdx(:, j) == edgeID - 1, 1);
                end
            end
            startNodeID = edgePile(edgeID, 1) + 1;
            endNodeID = edgePile(edgeID, 2) + 1;
            startX = flatNodeArray{1, startNodeID}.x;
            startY = flatNodeArray{1, startNodeID}.y;
            endX = flatNodeArray{1, endNodeID}.x;
            endY = flatNodeArray{1, endNodeID}.y;
            x = [startX, startX, endX, endX];   % 定义X轴的范围
            y = [startY, startY, endY, endY];   % 定义Y轴的范围
        
            level = multiEdgeID * (1 / newEdgeTmpCnt(singleEdgeID));
            z = [pileID + level - 1 / newEdgeTmpCnt(singleEdgeID), pileID + level, pileID + level, pileID + level - 1 / newEdgeTmpCnt(singleEdgeID)];
            % 使用fill函数填充颜色

            fill3(x, y, z, colors(i, :), 'FaceAlpha', 1);
            % 添加标记
            text((x(1) + x(4)) / 2, (y(1) + y(4)) / 2, (z(1) + z(2)) / 2, num2str(emphasizeTranID), 'HorizontalAlignment', 'center', 'Color', 'w');

        end


        maxVal = max(edgeStat(:, 3));
        cmap = turbo(maxVal + 1); % 定义颜色映射（可以根据实际需求选择其他颜色映射）
        hold on
        % 绘制每条边
        for j = 1 : size(edgeStat, 1)
            % 根据权值确定颜色深浅
            colorIndex = edgeStat(j, 3) + 1;

            color = cmap(colorIndex, :);
            lineWidth = 1 + 3 * ((colorIndex + 1) / (maxVal + 1));
            plot3([flatNodeArray{edgeStat(j, 1) + 1}.x, flatNodeArray{edgeStat(j, 2) + 1}.x], ...
            [flatNodeArray{edgeStat(j, 1) + 1}.y, flatNodeArray{edgeStat(j, 2) + 1}.y], ...
            [0 0], '-o', 'Color', color, 'LineWidth', lineWidth, ...
             'MarkerSize', 3,...
            'MarkerEdgeColor', 'b',...
            'MarkerFaceColor', 'b')
        end

        % 绘制每条边
        for j = 1 : size(edgeStat, 1)
            % 根据权值确定颜色深浅
            colorIndex = edgeStat(j, 3) + 1;

            color = cmap(colorIndex, :);
            lineWidth = 1 + 3 * ((colorIndex + 1) / (maxVal + 1));
            plot3([flatNodeArray{edgeStat(j, 1) + 1}.x, flatNodeArray{edgeStat(j, 2) + 1}.x], ...
            [flatNodeArray{edgeStat(j, 1) + 1}.y, flatNodeArray{edgeStat(j, 2) + 1}.y], ...
            [p p], '-o', 'Color', color, 'LineWidth', lineWidth, ...
             'MarkerSize', 3,...
            'MarkerEdgeColor', 'b',...
            'MarkerFaceColor', 'b')
        end
        
        axis off
        frame = getframe(basedStrategyVisulization);
        if breakFlag
            rgbpict(:, :, :, lineIndex) = frame2im(frame);
        else
            rgbpict(:, :, :, i - 1) = frame2im(frame);
        end
        hold off

        if breakFlag
            break;
        end
        
    end

    % 显示图表
    
    
    % 启用数据提示模式
    dcm = datacursormode(gcf);
    set(dcm, 'UpdateFcn', {@customGraphDatatip, nodeArray, args});
    set(gcf, 'Color', 'white');
    gifwrite(rgbpict, "Based_Strategy_Animation.gif", [0.4 * ones(1, lineIndex - 1) 1])

end