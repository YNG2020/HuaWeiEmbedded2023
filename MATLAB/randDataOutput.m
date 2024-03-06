function [] = randDataOutput(args, allEdge, buses)
    % 写入文件
    fileID = fopen('dataMATLAB.txt', 'w');
    
    % 写入第一行
    fprintf(fileID, '%d %d %d %d %d\n', args.N, args.M, args.T, args.P, args.D);
    
    % 写入边数据
    for i = 1 : args.M
        fprintf(fileID, '%d %d %d\n', allEdge(i, 1), allEdge(i, 2), allEdge(i, 3));
    end
    
    % 写入业务数据
    for j = 1 : args.T
        fprintf(fileID, '%d %d\n', buses(j, 1), buses(j, 2));
    end
    
    % 关闭文件
    fclose(fileID);
end